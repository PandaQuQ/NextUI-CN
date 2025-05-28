extern "C"
{
#include "msettings.h"

#include "defines.h"
#include "api.h"
#include "utils.h"
#include "i18n.h"
}

#include "wifimenu.hpp"
#include "keyboardprompt.hpp"

static int appQuit = false;
static bool appSuspend = false;

static void sigHandler(int sig)
{
    switch (sig)
    {
    case SIGINT:
    case SIGTERM:
        appQuit = true;
        break;
    case SIGSTOP:
        appSuspend = true;
        break;
    case SIGCONT:
        appSuspend = false;
        break;
    default:
        break;
    }
}

struct Context
{
    MenuList *menu;
    SDL_Surface *screen;
    int dirty;
    int show_setting;
};

// This is all the MinUiSettings stuff, for now just copied over from the old settings app

// Helper functions to get localized labels
std::vector<std::string> getLocalizedTimeoutLabels() {
    return {
        I18N_get("never"),
        I18N_get("5_seconds"),
        I18N_get("10_seconds"),
        I18N_get("15_seconds"),
        I18N_get("30_seconds"),
        I18N_get("45_seconds"),
        I18N_get("60_seconds"),
        I18N_get("90_seconds"),
        I18N_get("2_minutes"),
        I18N_get("4_minutes"),
        I18N_get("6_minutes"),
        I18N_get("10_minutes")
    };
}

std::vector<std::string> getLocalizedOnOffLabels() {
    return {I18N_get("off"), I18N_get("on")};
}

std::vector<std::string> getLocalizedScalingLabels() {
    return {I18N_get("fullscreen"), I18N_get("fit"), I18N_get("fill")};
}

std::vector<std::string> getLocalizedLanguageLabels() {
    std::vector<std::string> labels;
    for (int i = 0; i < LANG_COUNT; i++) {
        labels.push_back(I18N_getLanguageName((Language)i));
    }
    return labels;
}

std::vector<std::string> getLocalizedFontLabels() {
    return {I18N_get("font_1"), I18N_get("font_2")};
}

std::vector<std::string> getLocalizedVolumeLabels() {
    return {
        I18N_get("mute"), I18N_get("5_percent"), I18N_get("10_percent"), I18N_get("15_percent"), 
        I18N_get("20_percent"), I18N_get("25_percent"), I18N_get("30_percent"), I18N_get("35_percent"),
        I18N_get("40_percent"), I18N_get("45_percent"), I18N_get("50_percent"), I18N_get("55_percent"),
        I18N_get("60_percent"), I18N_get("65_percent"), I18N_get("70_percent"), I18N_get("75_percent"),
        I18N_get("80_percent"), I18N_get("85_percent"), I18N_get("90_percent"), I18N_get("95_percent"), 
        I18N_get("100_percent")
    };
}

std::vector<std::string> getLocalizedSaveFormatLabels() {
    return {
        I18N_get("minui_default"), I18N_get("retroarch_compressed"), 
        I18N_get("retroarch_uncompressed"), I18N_get("generic")
    };
}

std::vector<std::string> getLocalizedStateFormatLabels() {
    return {
        I18N_get("minui_default"), I18N_get("retroarch_compressed"), 
        I18N_get("retroarch_uncompressed")
    };
}

std::vector<std::string> getLocalizedMuteVolumeLabels() {
    std::vector<std::string> labels;
    labels.push_back(I18N_get("no_change"));
    auto volume_labels = getLocalizedVolumeLabels();
    labels.insert(labels.end(), volume_labels.begin(), volume_labels.end());
    return labels;
}

std::vector<std::string> getLocalizedMuteBrightnessLabels() {
    std::vector<std::string> labels;
    labels.push_back(I18N_get("no_change"));
    for (int i = 0; i <= 10; i++) {
        labels.push_back(std::to_string(i));
    }
    return labels;
}

std::vector<std::string> getLocalizedMuteColorTempLabels() {
    std::vector<std::string> labels;
    labels.push_back(I18N_get("no_change"));
    for (int i = 0; i <= 40; i++) {
        labels.push_back(std::to_string(i));
    }
    return labels;
}

std::vector<std::string> getLocalizedMuteContrastLabels() {
    std::vector<std::string> labels;
    labels.push_back(I18N_get("no_change"));
    for (int i = -4; i <= 5; i++) {
        labels.push_back(std::to_string(i));
    }
    return labels;
}

std::vector<std::string> getLocalizedMuteSaturationLabels() {
    std::vector<std::string> labels;
    labels.push_back(I18N_get("no_change"));
    for (int i = -5; i <= 5; i++) {
        labels.push_back(std::to_string(i));
    }
    return labels;
}

std::vector<std::string> getLocalizedMuteExposureLabels() {
    std::vector<std::string> labels;
    labels.push_back(I18N_get("no_change"));
    for (int i = -4; i <= 5; i++) {
        labels.push_back(std::to_string(i));
    }
    return labels;
}

static const std::vector<std::any> colors = {
    0x000022U, 0x000044U, 0x000066U, 0x000088U, 0x0000AAU, 0x0000CCU, 0x1e2329U, 0x3366FFU, 0x4D7AFFU, 0x6699FFU, 0x80B3FFU, 0x99CCFFU, 0xB3D9FFU,
    0x002222U, 0x004444U, 0x006666U, 0x008888U, 0x00AAAAU, 0x00CCCCU, 0x33FFFFU, 0x4DFFFFU, 0x66FFFFU, 0x80FFFFU, 0x99FFFFU, 0xB3FFFFU,
    0x002200U, 0x004400U, 0x006600U, 0x008800U, 0x00AA00U, 0x00CC00U, 0x33FF33U, 0x4DFF4DU, 0x66FF66U, 0x80FF80U, 0x99FF99U, 0xB3FFB3U,
    0x220022U, 0x440044U, 0x660066U, 0x880088U, 0x9B2257U, 0xAA00AAU, 0xCC00CCU, 0xFF33FFU, 0xFF4DFFU, 0xFF66FFU, 0xFF80FFU, 0xFF99FFU, 0xFFB3FFU,
    0x110022U, 0x220044U, 0x330066U, 0x440088U, 0x5500AAU, 0x6600CCU, 0x8833FFU, 0x994DFFU, 0xAA66FFU, 0xBB80FFU, 0xCC99FFU, 0xDDB3FFU,
    0x220000U, 0x440000U, 0x660000U, 0x880000U, 0xAA0000U, 0xCC0000U, 0xFF3333U, 0xFF4D4DU, 0xFF6666U, 0xFF8080U, 0xFF9999U, 0xFFB3B3U,
    0x222200U, 0x444400U, 0x666600U, 0x888800U, 0xAAAA00U, 0xCCCC00U, 0xFFFF33U, 0xFFFF4DU, 0xFFFF66U, 0xFFFF80U, 0xFFFF99U, 0xFFFFB3U,
    0x221100U, 0x442200U, 0x663300U, 0x884400U, 0xAA5500U, 0xCC6600U, 0xFF8833U, 0xFF994DU, 0xFFAA66U, 0xFFBB80U, 0xFFCC99U, 0xFFDDB3U,
    0x000000U, 0x141414U, 0x282828U, 0x3C3C3CU, 0x505050U, 0x646464U, 0x8C8C8CU, 0xA0A0A0U, 0xB4B4B4U, 0xC8C8C8U, 0xDCDCDCU, 0xFFFFFFU};
// all colors above but as strings
static const std::vector<std::string> color_strings = {
    "0x000022", "0x000044", "0x000066", "0x000088", "0x0000AA", "0x0000CC", "0x1E2329", "0x3366FF", "0x4D7AFF", "0x6699FF", "0x80B3FF", "0x99CCFF", "0xB3D9FF",
    "0x002222", "0x004444", "0x006666", "0x008888", "0x00AAAA", "0x00CCCC", "0x33FFFF", "0x4DFFFF", "0x66FFFF", "0x80FFFF", "0x99FFFF", "0xB3FFFF",
    "0x002200", "0x004400", "0x006600", "0x008800", "0x00AA00", "0x00CC00", "0x33FF33", "0x4DFF4D", "0x66FF66", "0x80FF80", "0x99FF99", "0xB3FFB3",
    "0x220022", "0x440044", "0x660066", "0x880088", "0x9B2257", "0xAA00AA", "0xCC00CC", "0xFF33FF", "0xFF4DFF", "0xFF66FF", "0xFF80FF", "0xFF99FF", "0xFFB3FF",
    "0x110022", "0x220044", "0x330066", "0x440088", "0x5500AA", "0x6600CC", "0x8833FF", "0x994DFF", "0xAA66FF", "0xBB80FF", "0xCC99FF", "0xDDB3FF",
    "0x220000", "0x440000", "0x660000", "0x880000", "0xAA0000", "0xCC0000", "0xFF3333", "0xFF4D4D", "0xFF6666", "0xFF8080", "0xFF9999", "0xFFB3B3",
    "0x222200", "0x444400", "0x666600", "0x888800", "0xAAAA00", "0xCCCC00", "0xFFFF33", "0xFFFF4D", "0xFFFF66", "0xFFFF80", "0xFFFF99", "0xFFFFB3",
    "0x221100", "0x442200", "0x663300", "0x884400", "0xAA5500", "0xCC6600", "0xFF8833", "0xFF994D", "0xFFAA66", "0xFFBB80", "0xFFCC99", "0xFFDDB3",
    "0x000000", "0x141414", "0x282828", "0x3C3C3C", "0x505050", "0x646464", "0x8C8C8C", "0xA0A0A0", "0xB4B4B4", "0xC8C8C8", "0xDCDCDC", "0xFFFFFF"};

static const std::vector<std::any> timeout_secs = {0U, 5U, 10U, 15U, 30U, 45U, 60U, 90U, 120U, 240U, 360U, 600U};

static const std::vector<std::any> scaling = {(int)GFX_SCALE_FULLSCREEN, (int)GFX_SCALE_FIT, (int)GFX_SCALE_FILL};

static const std::vector<std::any> language_values = {(int)LANG_EN_US, (int)LANG_ZH_CN, (int)LANG_JA_JP, (int)LANG_KO_KR};

int main(int argc, char *argv[])
{
    try
    {
        char version[128];
        PLAT_getOsVersionInfo(version, 128);
        LOG_info("This is TrimUI stock OS version %s\n", version);
        InitSettings();

        PWR_setCPUSpeed(CPU_SPEED_MENU);

        Context ctx = {0};
        ctx.dirty = 1;
        ctx.show_setting = 0;
        ctx.screen = GFX_init(MODE_MAIN);
        PAD_init();
        PWR_init();
        TIME_init();
        WIFI_init();

        signal(SIGINT, sigHandler);
        signal(SIGTERM, sigHandler);

        char timezones[MAX_TIMEZONES][MAX_TZ_LENGTH];
        int tz_count = 0;
        TIME_getTimezones(timezones, &tz_count);
        
        std::vector<std::any> tz_values;
        std::vector<std::string> tz_labels;
        for (int i = 0; i < tz_count; ++i) {
            //LOG_info("Timezone: %s\n", timezones[i]);
            tz_values.push_back(std::string(timezones[i]));
            // Todo: beautify, remove underscores and so on
            tz_labels.push_back(std::string(timezones[i]));
        }        // Get localized labels for the dynamic arrays
        auto font_labels = getLocalizedFontLabels();
        auto language_labels = getLocalizedLanguageLabels();
        auto on_off_labels = getLocalizedOnOffLabels();
        auto scaling_labels = getLocalizedScalingLabels();
        auto timeout_labels = getLocalizedTimeoutLabels();
        auto volume_labels = getLocalizedVolumeLabels();
        auto save_format_labels = getLocalizedSaveFormatLabels();
        auto state_format_labels = getLocalizedStateFormatLabels();
        auto mute_volume_labels = getLocalizedMuteVolumeLabels();
        auto mute_brightness_labels = getLocalizedMuteBrightnessLabels();
        auto mute_colortemp_labels = getLocalizedMuteColorTempLabels();
        auto mute_contrast_labels = getLocalizedMuteContrastLabels();
        auto mute_saturation_labels = getLocalizedMuteSaturationLabels();
        auto mute_exposure_labels = getLocalizedMuteExposureLabels();
        
        auto appearanceMenu = new MenuList(MenuItemType::Fixed, I18N_get("appearance"),
            {new MenuItem{ListItemType::Generic, I18N_get("font"), I18N_get("the_font_to_render_all_ui_text"), {0, 1}, font_labels, 
                []() -> std::any{ return CFG_getFontId(); },
                [](const std::any &value){ CFG_setFontId(std::any_cast<int>(value)); },
                []() { CFG_setFontId(CFG_DEFAULT_FONT_ID);}},
                new MenuItem{ListItemType::Generic, I18N_get("language_setting"), I18N_get("interface_language_setting"), language_values, language_labels, 
                []() -> std::any{ return CFG_getLanguage(); },
                [](const std::any &value){ CFG_setLanguage(std::any_cast<int>(value)); I18N_setLanguage((Language)std::any_cast<int>(value)); },
                []() { CFG_setLanguage(CFG_DEFAULT_LANGUAGE); I18N_setLanguage((Language)CFG_DEFAULT_LANGUAGE);}},
                new MenuItem{ListItemType::Color, I18N_get("main_color"), I18N_get("the_color_used_to_render_main_ui_elements"), colors, color_strings, 
                []() -> std::any{ return CFG_getColor(1); }, 
                [](const std::any &value){ CFG_setColor(1, std::any_cast<uint32_t>(value)); },
                []() { CFG_setColor(1, CFG_DEFAULT_COLOR1);}},
                new MenuItem{ListItemType::Color, I18N_get("primary_accent_color"), I18N_get("the_color_used_to_highlight_important_things_in_the_user_interface"), colors, color_strings, 
                []() -> std::any{ return CFG_getColor(2); }, 
                [](const std::any &value){ CFG_setColor(2, std::any_cast<uint32_t>(value)); },
                []() { CFG_setColor(2, CFG_DEFAULT_COLOR2);}},
                new MenuItem{ListItemType::Color, I18N_get("secondary_accent_color"), I18N_get("a_secondary_highlight_color"), colors, color_strings, 
                []() -> std::any{ return CFG_getColor(3); }, 
                [](const std::any &value){ CFG_setColor(3, std::any_cast<uint32_t>(value)); },
                []() { CFG_setColor(3, CFG_DEFAULT_COLOR3);}},
                new MenuItem{ListItemType::Color, I18N_get("hint_info_color"), I18N_get("color_for_button_hints_and_info"), colors, color_strings, 
                []() -> std::any{ return CFG_getColor(6); }, 
                [](const std::any &value){ CFG_setColor(6, std::any_cast<uint32_t>(value)); },
                []() { CFG_setColor(6, CFG_DEFAULT_COLOR6);}},
                new MenuItem{ListItemType::Color, I18N_get("list_text"), I18N_get("list_text_color"), colors, color_strings, 
                []() -> std::any{ return CFG_getColor(4); }, 
                [](const std::any &value){ CFG_setColor(4, std::any_cast<uint32_t>(value)); },
                []() { CFG_setColor(4, CFG_DEFAULT_COLOR4);}},
                new MenuItem{ListItemType::Color, I18N_get("list_text_selected"), I18N_get("list_selected_text_color"), colors, color_strings, 
                []() -> std::any { return CFG_getColor(5); }, 
                [](const std::any &value) { CFG_setColor(5, std::any_cast<uint32_t>(value)); },
                []() { CFG_setColor(5, CFG_DEFAULT_COLOR5);}},
                new MenuItem{ListItemType::Generic, I18N_get("show_battery_percentage"), I18N_get("show_battery_level_as_percent_in_the_status_pill"), {false, true}, on_off_labels, 
                []() -> std::any { return CFG_getShowBatteryPercent(); },
                [](const std::any &value) { CFG_setShowBatteryPercent(std::any_cast<bool>(value)); },
                []() { CFG_setShowBatteryPercent(CFG_DEFAULT_SHOWBATTERYPERCENT);}},
                new MenuItem{ListItemType::Generic, I18N_get("show_menu_animations"), I18N_get("enable_or_disable_menu_animations"), {false, true}, on_off_labels, 
                []() -> std::any{ return CFG_getMenuAnimations(); },
                [](const std::any &value) { CFG_setMenuAnimations(std::any_cast<bool>(value)); },
                []() { CFG_setMenuAnimations(CFG_DEFAULT_SHOWMENUANIMATIONS);}},
                new MenuItem{ListItemType::Generic, I18N_get("show_menu_transitions"), I18N_get("enable_or_disable_animated_transitions"), {false, true}, on_off_labels, 
                []() -> std::any{ return CFG_getMenuTransitions(); },
                [](const std::any &value) { CFG_setMenuTransitions(std::any_cast<bool>(value)); },
                []() { CFG_setMenuTransitions(CFG_DEFAULT_SHOWMENUTRANSITIONS);}},
                new MenuItem{ListItemType::Generic, I18N_get("game_art_corner_radius"), I18N_get("set_the_radius_for_the_rounded_corners_of_game_art"), 0, 24, "px",
                []() -> std::any{ return CFG_getThumbnailRadius(); }, 
                [](const std::any &value) { CFG_setThumbnailRadius(std::any_cast<int>(value)); },
                []() { CFG_setThumbnailRadius(CFG_DEFAULT_THUMBRADIUS);}},
                new MenuItem{ListItemType::Generic, I18N_get("game_art_width"), I18N_get("set_the_percentage_of_screen_width_used_for_game_art"), 5, 100, "%",
                []() -> std::any{ return (int)(CFG_getGameArtWidth() * 100); }, 
                [](const std::any &value) { CFG_setGameArtWidth((double)std::any_cast<int>(value) / 100.0); },
                []() { CFG_setGameArtWidth(CFG_DEFAULT_GAMEARTWIDTH);}},
                new MenuItem{ListItemType::Generic, I18N_get("show_recents"), I18N_get("show_recently_played_menu_entry"), {false, true}, on_off_labels,
                []() -> std::any { return CFG_getShowRecents(); },
                [](const std::any &value) { CFG_setShowRecents(std::any_cast<bool>(value)); },
                []() { CFG_setShowRecents(CFG_DEFAULT_SHOWRECENTS);}},                new MenuItem{ListItemType::Generic, I18N_get("show_game_art"), I18N_get("show_game_artwork_in_the_main_menu"), {false, true}, on_off_labels, []() -> std::any
                { return CFG_getShowGameArt(); },
                [](const std::any &value)
                { CFG_setShowGameArt(std::any_cast<bool>(value)); },
                []() { CFG_setShowGameArt(CFG_DEFAULT_SHOWGAMEART);}},
                new MenuItem{ListItemType::Generic, I18N_get("use_folder_background_for_roms"), I18N_get("if_enabled_used_the_emulator_background_image"), {false, true}, on_off_labels, []() -> std::any
                { return CFG_getRomsUseFolderBackground(); },
                [](const std::any &value)
                { CFG_setRomsUseFolderBackground(std::any_cast<bool>(value)); },
                []() { CFG_setRomsUseFolderBackground(CFG_DEFAULT_ROMSUSEFOLDERBACKGROUND);}},
                // not needed anymore
                // new MenuItem{ListItemType::Generic, "Game switcher scaling", "The scaling algorithm used to display the savegame image.", scaling, scaling_strings, []() -> std::any
                // { return CFG_getGameSwitcherScaling(); },
                // [](const std::any &value)
                // { CFG_setGameSwitcherScaling(std::any_cast<int>(value)); },
                // []() { CFG_setGameSwitcherScaling(CFG_DEFAULT_GAMESWITCHERSCALING);}},

                new MenuItem{ListItemType::Button, I18N_get("reset_to_defaults"), I18N_get("resets_all_options_in_this_menu_to_their_default_values"), ResetCurrentMenu},
        });        auto displayMenu = new MenuList(MenuItemType::Fixed, I18N_get("display"),
        {
            new MenuItem{ListItemType::Generic, I18N_get("brightness"), I18N_get("display_brightness_0_to_10"), 0, 10, "",[]() -> std::any
            { return GetBrightness(); }, [](const std::any &value)
            { SetBrightness(std::any_cast<int>(value)); },
            []() { SetBrightness(SETTINGS_DEFAULT_BRIGHTNESS);}},
            new MenuItem{ListItemType::Generic, I18N_get("color_temperature"), I18N_get("color_temperature_0_to_40"), 0, 40, "",[]() -> std::any
            { return GetColortemp(); }, [](const std::any &value)
            { SetColortemp(std::any_cast<int>(value)); },
            []() { SetColortemp(SETTINGS_DEFAULT_COLORTEMP);}},
            new MenuItem{ListItemType::Generic, I18N_get("contrast"), I18N_get("contrast_enhancement_minus4_to_5"), -4, 5, "",[]() -> std::any
            { return GetContrast(); }, [](const std::any &value)
            { SetContrast(std::any_cast<int>(value)); },
            []() { SetContrast(SETTINGS_DEFAULT_CONTRAST);}},            new MenuItem{ListItemType::Generic, I18N_get("saturation"), I18N_get("saturation_enhancement_minus5_to_5"), -5, 5, "",[]() -> std::any
            { return GetSaturation(); }, [](const std::any &value)
            { SetSaturation(std::any_cast<int>(value)); },
            []() { SetSaturation(SETTINGS_DEFAULT_SATURATION);}},
            new MenuItem{ListItemType::Generic, I18N_get("exposure"), I18N_get("exposure_enhancement_minus4_to_5"), -4, 5, "",[]() -> std::any
            { return GetExposure(); }, [](const std::any &value)
            { SetExposure(std::any_cast<int>(value)); },            []() { SetExposure(SETTINGS_DEFAULT_EXPOSURE);}},

            new MenuItem{ListItemType::Button, I18N_get("reset_to_defaults"), I18N_get("resets_all_options_in_this_menu_to_their_default_values"), ResetCurrentMenu},
        });        auto systemMenu = new MenuList(MenuItemType::Fixed, I18N_get("system"),
        {
            new MenuItem{ListItemType::Generic, I18N_get("volume"), I18N_get("speaker_volume"),
            {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20},
            volume_labels,
            []() -> std::any{ return GetVolume(); }, [](const std::any &value)
            { SetVolume(std::any_cast<int>(value)); },
            []() { SetVolume(SETTINGS_DEFAULT_VOLUME);}},
            new MenuItem{ListItemType::Generic, I18N_get("screen_timeout"), I18N_get("time_before_screen_turns_off_0_600s"), timeout_secs, timeout_labels, []() -> std::any
            { return CFG_getScreenTimeoutSecs(); }, [](const std::any &value)
            { CFG_setScreenTimeoutSecs(std::any_cast<uint32_t>(value)); },
            []() { CFG_setScreenTimeoutSecs(CFG_DEFAULT_SCREENTIMEOUTSECS);}},
            new MenuItem{ListItemType::Generic, I18N_get("suspend_timeout"), I18N_get("time_before_device_goes_to_sleep_0_600s"), timeout_secs, timeout_labels, []() -> std::any
            { return CFG_getSuspendTimeoutSecs(); }, [](const std::any &value)
            { CFG_setSuspendTimeoutSecs(std::any_cast<uint32_t>(value)); },
            []() { CFG_setSuspendTimeoutSecs(CFG_DEFAULT_SUSPENDTIMEOUTSECS);}},
            new MenuItem{ListItemType::Generic, I18N_get("haptic_feedback"), I18N_get("enable_or_disable_haptic_feedback_on_certain_actions_in_the_os"), {false, true}, on_off_labels, []() -> std::any
            { return CFG_getHaptics(); }, [](const std::any &value)
            { CFG_setHaptics(std::any_cast<bool>(value)); },
            []() { CFG_setHaptics(CFG_DEFAULT_HAPTICS);}},
            new MenuItem{ListItemType::Generic, I18N_get("show_24h_time_format"), I18N_get("show_clock_in_the_24hrs_time_format"), {false, true}, on_off_labels, []() -> std::any
            { return CFG_getClock24H(); },
            [](const std::any &value)
            { CFG_setClock24H(std::any_cast<bool>(value)); },
            []() { CFG_setClock24H(CFG_DEFAULT_CLOCK24H);}},
            new MenuItem{ListItemType::Generic, I18N_get("show_clock"), I18N_get("show_clock_in_the_status_pill"), {false, true}, on_off_labels, []() -> std::any
            { return CFG_getShowClock(); },
            [](const std::any &value)
            { CFG_setShowClock(std::any_cast<bool>(value)); },
            []() { CFG_setShowClock(CFG_DEFAULT_SHOWCLOCK);}},
            new MenuItem{ListItemType::Generic, I18N_get("set_time_and_date_automatically"), I18N_get("automatically_adjust_system_time_with_ntp"), {false, true}, on_off_labels, []() -> std::any
            { return TIME_getNetworkTimeSync(); }, [](const std::any &value)
            { TIME_setNetworkTimeSync(std::any_cast<bool>(value)); },
            []() { TIME_setNetworkTimeSync(false);}}, // default from stock
            new MenuItem{ListItemType::Generic, I18N_get("timezone"), I18N_get("your_timezone"), tz_values, tz_labels, []() -> std::any
            { return std::string(TIME_getCurrentTimezone()); }, [](const std::any &value)
            { TIME_setCurrentTimezone(std::any_cast<std::string>(value).c_str()); },
            []() { TIME_setCurrentTimezone("Asia/Shanghai");}}, // default from Stock
            new MenuItem{ListItemType::Generic, I18N_get("save_format"), I18N_get("save_format_description"),
            {(int)SAVE_FORMAT_SAV, (int)SAVE_FORMAT_SRM, (int)SAVE_FORMAT_SRM_UNCOMPRESSED, (int)SAVE_FORMAT_GEN},
            save_format_labels, []() -> std::any
            { return CFG_getSaveFormat(); }, [](const std::any &value)
            { CFG_setSaveFormat(std::any_cast<int>(value)); },
            []() { CFG_setSaveFormat(CFG_DEFAULT_SAVEFORMAT);}},
            new MenuItem{ListItemType::Generic, I18N_get("save_state_format"), I18N_get("save_state_format_description"),
            {(int)STATE_FORMAT_SAV, (int)STATE_FORMAT_SRM, (int)STATE_FORMAT_SRM_UNCOMPRESSED},
            state_format_labels, []() -> std::any
            { return CFG_getStateFormat(); }, [](const std::any &value)
            { CFG_setStateFormat(std::any_cast<int>(value)); },
            []() { CFG_setStateFormat(CFG_DEFAULT_STATEFORMAT);}},

            new MenuItem{ListItemType::Button, I18N_get("reset_to_defaults"), I18N_get("resets_all_options_in_this_menu_to_their_default_values"), ResetCurrentMenu},
        });        auto muteMenu = new MenuList(MenuItemType::Fixed, I18N_get("fn_switch_settings"),
        {
            new MenuItem{ListItemType::Generic, I18N_get("adjust_volume_when_enabled"), I18N_get("speaker_volume_0_20"), 
            {(int)SETTINGS_DEFAULT_MUTE_NO_CHANGE, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20}, 
            mute_volume_labels, 
            []() -> std::any { return GetMutedVolume(); },
            [](const std::any &value) { SetMutedVolume(std::any_cast<int>(value)); },
            []() { SetMutedVolume(0); }},
            new MenuItem{ListItemType::Generic, I18N_get("fn_key_disable_leds"), I18N_get("fn_key_will_also_disable_leds"), {false, true}, on_off_labels,
            []() -> std::any { return CFG_getMuteLEDs(); },
            [](const std::any &value) { CFG_setMuteLEDs(std::any_cast<bool>(value)); },
            []() { CFG_setMuteLEDs(CFG_DEFAULT_MUTELEDS); }},
            new MenuItem{ListItemType::Generic, I18N_get("adjust_brightness_when_enabled"), I18N_get("display_brightness_0_to_10"), 
            {(int)SETTINGS_DEFAULT_MUTE_NO_CHANGE, 0,1,2,3,4,5,6,7,8,9,10}, 
            mute_brightness_labels,
            []() -> std::any { return GetMutedBrightness(); }, [](const std::any &value)
            { SetMutedBrightness(std::any_cast<int>(value)); },
            []() { SetMutedBrightness(SETTINGS_DEFAULT_MUTE_NO_CHANGE);}},
            new MenuItem{ListItemType::Generic, I18N_get("adjust_color_temperature_when_enabled"), I18N_get("color_temperature_0_to_40"), 
            {(int)SETTINGS_DEFAULT_MUTE_NO_CHANGE, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40}, 
            mute_colortemp_labels,
            []() -> std::any{ return GetMutedColortemp(); }, [](const std::any &value)
            { SetMutedColortemp(std::any_cast<int>(value)); },
            []() { SetMutedColortemp(SETTINGS_DEFAULT_MUTE_NO_CHANGE);}},
            new MenuItem{ListItemType::Generic, I18N_get("adjust_contrast_when_enabled"), I18N_get("contrast_enhancement_minus4_to_5"), 
            {(int)SETTINGS_DEFAULT_MUTE_NO_CHANGE, -4,-3,-2,-1,0,1,2,3,4,5}, 
            mute_contrast_labels, 
            []() -> std::any  { return GetMutedContrast(); }, [](const std::any &value)
            { SetMutedContrast(std::any_cast<int>(value)); },
            []() { SetMutedContrast(SETTINGS_DEFAULT_MUTE_NO_CHANGE);}},
            new MenuItem{ListItemType::Generic, I18N_get("adjust_saturation_when_enabled"), I18N_get("saturation_enhancement_minus5_to_5"), 
            {(int)SETTINGS_DEFAULT_MUTE_NO_CHANGE, -5,-4,-3,-2,-1,0,1,2,3,4,5}, 
            mute_saturation_labels, 
            []() -> std::any{ return GetMutedSaturation(); }, [](const std::any &value)
            { SetMutedSaturation(std::any_cast<int>(value)); },
            []() { SetMutedSaturation(SETTINGS_DEFAULT_MUTE_NO_CHANGE);}},
            new MenuItem{ListItemType::Generic, I18N_get("adjust_exposure_when_enabled"), I18N_get("exposure_enhancement_minus4_to_5"), 
            {(int)SETTINGS_DEFAULT_MUTE_NO_CHANGE, -4,-3,-2,-1,0,1,2,3,4,5}, 
            mute_exposure_labels, 
            []() -> std::any  { return GetMutedExposure(); }, [](const std::any &value)
            { SetMutedExposure(std::any_cast<int>(value)); },
            []() { SetMutedExposure(SETTINGS_DEFAULT_MUTE_NO_CHANGE);}},

            new MenuItem{ListItemType::Button, I18N_get("reset_to_defaults"), I18N_get("resets_all_options_in_this_menu_to_their_default_values"), ResetCurrentMenu},
        });

        // TODO: check WIFI_supported(), hide menu otherwise
        auto networkMenu = new Wifi::Menu(appQuit);        ctx.menu = new MenuList(MenuItemType::List, I18N_get("main"),
        {
            new MenuItem{ListItemType::Generic, I18N_get("appearance"), I18N_get("ui_customization"), {}, {}, nullptr, nullptr, DeferToSubmenu, appearanceMenu},
            new MenuItem{ListItemType::Generic, I18N_get("display"), "", {}, {}, nullptr, nullptr, DeferToSubmenu, displayMenu},
            new MenuItem{ListItemType::Generic, I18N_get("system"), "", {}, {}, nullptr, nullptr, DeferToSubmenu, systemMenu},
            new MenuItem{ListItemType::Generic, I18N_get("fn_switch"), I18N_get("fn_switch_settings"), {}, {}, nullptr, nullptr, DeferToSubmenu, muteMenu},
            new MenuItem{ListItemType::Generic, I18N_get("network"), "", {}, {}, nullptr, nullptr, DeferToSubmenu, networkMenu},
        });

        //ctx.menu = new KeyboardPrompt("test", [](MenuItem &itm) -> InputReactionHint
        //                              {   
        //    LOG_info("Keyboard text: %s\n", itm.getName().c_str());
        //    return NoOp; });

        const bool showTitle = false;
        const bool showIndicator = true;
        const bool showHints = false;

        SDL_Surface* bgbmp = IMG_Load(SDCARD_PATH "/bg.png");
        SDL_Surface* convertedbg = SDL_ConvertSurfaceFormat(bgbmp, SDL_PIXELFORMAT_RGB565, 0);
        if (convertedbg) {
            SDL_FreeSurface(bgbmp); 
            SDL_Surface* scaled = SDL_CreateRGBSurfaceWithFormat(0, ctx.screen->w, ctx.screen->h, 32, SDL_PIXELFORMAT_RGB565);
            GFX_blitScaleToFill(convertedbg, scaled);
            bgbmp = scaled;
        }

        // main content (list)
        // PADDING all around
        SDL_Rect listRect = {SCALE1(PADDING), SCALE1(PADDING), ctx.screen->w - SCALE1(PADDING * 2), ctx.screen->h - SCALE1(PADDING * 2)};
        // PILL_SIZE above (if showing title)
        if (showTitle || showIndicator)
            listRect = dy(listRect, SCALE1(PILL_SIZE));
        // BUTTON_SIZE below (if showing hints)
        if (showHints)
            listRect.h -= SCALE1(BUTTON_SIZE);
        ctx.menu->performLayout(listRect);

        while (!appQuit)
        {
            GFX_startFrame();
            uint32_t now = SDL_GetTicks();
            PAD_poll();

            ctx.menu->handleInput(ctx.dirty, appQuit);

            PWR_update(&ctx.dirty, &ctx.show_setting, nullptr, nullptr);

            if (ctx.dirty)
            {
                GFX_clear(ctx.screen);
                if(bgbmp) {
                    SDL_Rect image_rect = {0, 0, ctx.screen->w, ctx.screen->h};
                    SDL_BlitSurface(bgbmp, NULL, ctx.screen, &image_rect);
                }

                int ow = 0;

                // indicator area top right
                if (showIndicator)
                {
                    ow = GFX_blitHardwareGroup(ctx.screen, ctx.show_setting);
                }
                int max_width = ctx.screen->w - SCALE1(PADDING * 2) - ow;

                // title pill
                if (showTitle)
                {
                    char display_name[256];
                    int text_width = GFX_truncateText(font.large, "Some title", display_name, max_width, SCALE1(BUTTON_PADDING * 2));
                    max_width = MIN(max_width, text_width);

                    SDL_Surface *text;
                    text = TTF_RenderUTF8_Blended(font.large, display_name, COLOR_WHITE);
                    SDL_Rect target = {SCALE1(PADDING), SCALE1(PADDING), max_width, SCALE1(PILL_SIZE)};
                    GFX_blitPillLight(ASSET_WHITE_PILL, ctx.screen, &target);
                    SDL_BlitSurfaceCPP(text, {0, 0, max_width - SCALE1(BUTTON_PADDING * 2), text->h}, ctx.screen, {SCALE1(PADDING + BUTTON_PADDING), SCALE1(PADDING + 4)});
                    SDL_FreeSurface(text);
                }

                // bottom area, button hints
                if (showHints)
                {
                    if (ctx.show_setting && !GetHDMI())
                        GFX_blitHardwareHints(ctx.screen, ctx.show_setting);
                    else
                    {
                        char *hints[] = {(char *)(I18N_get("MENU")), (char *)(I18N_get("sleep")), NULL};
                        GFX_blitButtonGroup(hints, 0, ctx.screen, 0);
                    }
                    char *hints[] = {(char *)(I18N_get("B")), (char *)(I18N_get("back")), (char *)(I18N_get("A")), (char *)(I18N_get("ok")), NULL};
                    GFX_blitButtonGroup(hints, 1, ctx.screen, 1);
                }

                ctx.menu->draw(ctx.screen, listRect);

                // present
                GFX_flip(ctx.screen);
                ctx.dirty = false;

                // hdmimon();
            }
        }

        delete ctx.menu;
        delete appearanceMenu;
        delete systemMenu;
        ctx.menu = NULL;

        QuitSettings();
        PWR_quit();
        PAD_quit();
        GFX_quit();

        return EXIT_SUCCESS;
    }
    catch (const std::exception &e)
    {
        LOG_error("%s", e.what());
        QuitSettings();
        PWR_quit();
        PAD_quit();
        GFX_quit();

        return EXIT_FAILURE;
    }
}
