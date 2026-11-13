#include "palettemenu.hpp"
#include "i18n.h"

extern "C" {
#include "config.h"
#include "palette.h"
}

namespace {

// Menu row for picking a predefined color palette. Left/Right cycles through the
// available palettes (plus a "Custom" entry) and applies immediately; Confirm opens
// a submenu list. The displayed value is read live from CFG_getPaletteName() so it
// reflects "Custom" the moment an individual color is edited elsewhere. The empty
// string is the "Custom" value.
class PaletteMenuItem : public MenuItem
{
public:
    using MenuItem::MenuItem;

    const std::string getLabel() const override
    {
        std::string name;
        if (on_get)
        {
            auto v = on_get();
            if (v.has_value())
                name = std::any_cast<std::string>(v);
        }
        if (name.empty())
            return TR("settings.palette.custom");
        // Show the persisted palette only if it is still available this session.
        for (const auto &v : getValues())
            if (std::any_cast<std::string>(v) == name)
                return name;
        return TR("settings.palette.custom");
    }

    InputReactionHint handleInput(int &dirty) override
    {
        // Re-sync from the live palette name so Left/Right cycle from the correct
        // position even after a color edit flipped us to Custom.
        if (!isDeferred())
            reselect();
        return MenuItem::handleInput(dirty);
    }
};

} // namespace

AbstractMenuItem* buildPaletteMenuItem()
{
    // Enumerate once when the menu is built.
    std::vector<ColorPalette> palettes(64);
    int paletteCount = PALETTE_enumerate(palettes.data(), (int)palettes.size());
    palettes.resize(paletteCount);

    // Cycle values: "" == Custom, followed by each palette name.
    std::vector<std::any> palette_values;
    std::vector<std::string> palette_labels;
    palette_values.push_back(std::string(""));
    palette_labels.push_back(TR("settings.palette.custom"));
    for (const auto &p : palettes) {
        palette_values.push_back(std::string(p.name));
        palette_labels.push_back(p.name);
    }

    // Submenu: a plain list; selecting a row applies it and closes.
    std::vector<AbstractMenuItem *> paletteSubItems;
    paletteSubItems.push_back(new MenuItem{ListItemType::Button, TR("settings.palette.custom"), "",
        [](AbstractMenuItem &) -> InputReactionHint { CFG_selectCustomPalette(); return Exit; }});
    for (const auto &p : palettes) {
        ColorPalette palette = p; // capture a copy for the closure
        paletteSubItems.push_back(new MenuItem{ListItemType::Button, p.name, "",
            [palette](AbstractMenuItem &) -> InputReactionHint { PALETTE_apply(&palette); return Exit; }});
    }
    auto *paletteSubmenu = new MenuList(MenuItemType::Fixed, TR("settings.palette"), std::move(paletteSubItems));

    // Selector row. on_set applies the palette named by the cycled value ("" == Custom).
    std::vector<ColorPalette> palettesForSetter = palettes;
    return new PaletteMenuItem{ListItemType::Generic, TR("settings.palette"),
        TR("settings.palette.desc"),
        palette_values, palette_labels,
        []() -> std::any { return std::string(CFG_getPaletteName()); },
        [palettesForSetter](const std::any &v) {
            std::string name = std::any_cast<std::string>(v);
            if (name.empty()) { CFG_selectCustomPalette(); return; }
            for (const auto &p : palettesForSetter)
                if (name == p.name) { PALETTE_apply(&p); return; }
        },
        []() { CFG_clearPalette(); }, // "Reset to defaults": detach only, colors reset by each color item's own on_reset
        DeferToSubmenu, paletteSubmenu};
}
