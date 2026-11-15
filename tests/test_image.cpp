#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/image.hpp"
#include "../src/gui_manager.hpp"
#include "../src/theme.hpp"
#include "../src/widget_factory.hpp"

TEST_CASE("Image widget basics", "[image]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("starts empty with Fit scaling") {
        Image img(manager, 10, 20, 100, 50);
        REQUIRE(img.getX() == 10);
        REQUIRE(img.getY() == 20);
        REQUIRE(img.getWidth() == 100);
        REQUIRE(img.getHeight() == 50);
        REQUIRE_FALSE(img.hasImage());
        REQUIRE(img.getScaleMode() == Image::ScaleMode::Fit);
        REQUIRE_FALSE(img.isNearestScaling());
        REQUIRE(img.getImageSize() == std::pair<int, int>{0, 0});
    }

    SECTION("setImage(path) loads through TextureManager cache") {
        Image img(manager, 0, 0, 100, 100);
        auto tex = helper.makeStubTexture(80, 30);
        helper.primeTextureAs("stub_image.png", tex);

        REQUIRE(img.setImage("stub_image.png"));
        REQUIRE(img.hasImage());
        REQUIRE(img.getTexture() == tex);  // shared cache entry, no copy
        REQUIRE(img.getImagePath() == "stub_image.png");
        REQUIRE(img.getImageSize() == std::pair<int, int>{80, 30});
    }

    SECTION("setImage with a missing path clears the image") {
        Image img(manager, 0, 0, 100, 100);
        helper.primeTextureAs("present.png", helper.makeStubTexture(10, 10));
        REQUIRE(img.setImage("present.png"));
        REQUIRE_FALSE(img.setImage("definitely_missing_xyz.png"));
        REQUIRE_FALSE(img.hasImage());
        REQUIRE(img.getImageSize() == std::pair<int, int>{0, 0});
    }

    SECTION("setImage(SharedTexture) takes a caller-provided texture") {
        Image img(manager, 0, 0, 100, 100);
        auto tex = helper.makeStubTexture(64, 32);
        img.setImage(tex);
        REQUIRE(img.hasImage());
        REQUIRE(img.getTexture() == tex);
        REQUIRE(img.getImagePath().empty());
    }

    SECTION("clearImage drops the texture") {
        Image img(manager, 0, 0, 100, 100);
        img.setImage(helper.makeStubTexture(16, 16));
        REQUIRE(img.hasImage());
        img.clearImage();
        REQUIRE_FALSE(img.hasImage());
        img.clearImage();  // idempotent
    }

    SECTION("sizeToImage resizes to native dimensions, no-op when empty") {
        Image img(manager, 0, 0, 10, 10);
        img.sizeToImage();
        REQUIRE(img.getWidth() == 10);

        img.setImage(helper.makeStubTexture(120, 45));
        img.sizeToImage();
        REQUIRE(img.getWidth() == 120);
        REQUIRE(img.getHeight() == 45);
    }

    SECTION("setNearestScaling applies SDL texture scale mode") {
        Image img(manager, 0, 0, 100, 100);
        auto tex = helper.makeStubTexture(32, 32);
        img.setImage(tex);

        SDL_ScaleMode mode = SDL_SCALEMODE_INVALID;
        SDL_GetTextureScaleMode(tex.get(), &mode);
        REQUIRE(mode == SDL_SCALEMODE_LINEAR);

        img.setNearestScaling(true);
        REQUIRE(img.isNearestScaling());
        SDL_GetTextureScaleMode(tex.get(), &mode);
        REQUIRE(mode == SDL_SCALEMODE_NEAREST);

        img.setNearestScaling(false);
        SDL_GetTextureScaleMode(tex.get(), &mode);
        REQUIRE(mode == SDL_SCALEMODE_LINEAR);
    }

    SECTION("scale mode round-trips") {
        Image img(manager, 0, 0, 100, 100);
        img.setScaleMode(Image::ScaleMode::Stretch);
        REQUIRE(img.getScaleMode() == Image::ScaleMode::Stretch);
        img.setScaleMode(Image::ScaleMode::Center);
        REQUIRE(img.getScaleMode() == Image::ScaleMode::Center);
        img.setScaleMode(Image::ScaleMode::None);
        REQUIRE(img.getScaleMode() == Image::ScaleMode::None);
        img.setScaleMode(Image::ScaleMode::Fit);
        REQUIRE(img.getScaleMode() == Image::ScaleMode::Fit);
    }
}

TEST_CASE("Image renders its texture with every scale mode", "[image][render][pixel]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    manager.setTheme(Theme::createDefaultTheme());
    manager.handleResize(320, 240);

    auto tex = helper.makeStubTexture(20, 10, 255, 0, 0, 255);
    auto img = std::make_unique<Image>(manager, 10, 10, 40, 40);
    Image* image = img.get();
    image->setImage(tex);
    manager.addElement(std::move(img));

    auto readPixel = [&](int x, int y) {
        SDL_Rect r{x, y, 1, 1};
        SDL_Surface* surf = SDL_RenderReadPixels(helper.getRenderer(), &r);
        REQUIRE(surf != nullptr);
        auto result = readbackPixel(surf);
        SDL_DestroySurface(surf);
        return result;
    };
    auto isRed = [](const std::array<Uint8, 4>& px) {
        return px[0] == 255 && px[1] == 0 && px[2] == 0 && px[3] == 255;
    };
    auto isTransparent = [](const std::array<Uint8, 4>& px) { return px[3] == 0; };

    SECTION("Fit contains and centers the image") {
        manager.render();
        // 20x10 scaled 2x to 40x20, vertically centered at y = 10 + 10.
        REQUIRE(isRed(readPixel(30, 30)));
        REQUIRE(isTransparent(readPixel(30, 12)));
    }

    SECTION("Stretch fills the whole element") {
        image->setScaleMode(Image::ScaleMode::Stretch);
        manager.render();
        REQUIRE(isRed(readPixel(12, 12)));
        REQUIRE(isRed(readPixel(48, 48)));
    }

    SECTION("Center keeps native size and centers") {
        image->setScaleMode(Image::ScaleMode::Center);
        manager.render();
        // 20x10 at (10 + 10, 10 + 15) = (20, 25).
        REQUIRE(isRed(readPixel(25, 27)));
        REQUIRE(isTransparent(readPixel(12, 12)));
    }

    SECTION("None draws at native size in the top-left corner") {
        image->setScaleMode(Image::ScaleMode::None);
        manager.render();
        REQUIRE(isRed(readPixel(12, 12)));
        REQUIRE(isTransparent(readPixel(35, 35)));
    }

    SECTION("clearing the image removes the pixels") {
        image->clearImage();
        manager.render();
        REQUIRE(isTransparent(readPixel(30, 30)));
    }
}

TEST_CASE("Image is registered in WidgetFactory", "[image][factory]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    REQUIRE(WidgetFactory::isKnownType("Image"));
    REQUIRE(WidgetFactory::defaultSize("Image") == std::pair<int, int>{100, 100});

    SECTION("createBare returns an Image with defaults") {
        auto w = WidgetFactory::createBare(manager, "Image", 5, 6, 70, 80);
        REQUIRE(w != nullptr);
        REQUIRE(w->getComponentTypeId() == ComponentType::Image);
        auto* img = dynamic_cast<Image*>(w.get());
        REQUIRE(img != nullptr);
        REQUIRE(img->getScaleMode() == Image::ScaleMode::Fit);
        REQUIRE_FALSE(img->hasImage());
    }

    SECTION("create applies path, scale mode and sampling") {
        auto tex = helper.makeStubTexture(48, 24);
        helper.primeTextureAs("factory_image.png", tex);

        WidgetProps props;
        props.x = 1;
        props.y = 2;
        props.w = 96;
        props.h = 48;
        props.path = "factory_image.png";
        props.scaleMode = "Stretch";
        props.nearestScaling = true;

        auto w = WidgetFactory::create(manager, "Image", props);
        REQUIRE(w != nullptr);
        auto* img = dynamic_cast<Image*>(w.get());
        REQUIRE(img != nullptr);
        REQUIRE(img->hasImage());
        REQUIRE(img->getImageSize() == std::pair<int, int>{48, 24});
        REQUIRE(img->getScaleMode() == Image::ScaleMode::Stretch);
        REQUIRE(img->isNearestScaling());
    }
}
