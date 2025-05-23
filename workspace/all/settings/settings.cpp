extern "C"
{
#include "msettings.h"

#include "defines.h"
#include "api.h"
#include "utils.h"
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

static const std::vector<std::string> font_names = {"寒蝉半圆体", "寒蝉全圆体"};

static const std::vector<std::any> timeout_secs = {0U, 5U, 10U, 15U, 30U, 45U, 60U, 90U, 120U, 240U, 360U, 600U};
static const std::vector<std::string> timeout_labels = {"从不", "5秒", "10秒", "15秒", "30秒", "45秒", "60秒", "90秒", "2分钟", "4分钟", "6分钟", "10分钟"};

static const std::vector<std::string> on_off = {"关闭", "开启"};

static const std::vector<std::string> scaling_strings = {"全屏", "适应", "填充"};
static const std::vector<std::any> scaling = {(int)GFX_SCALE_FULLSCREEN, (int)GFX_SCALE_FIT, (int)GFX_SCALE_FILL};

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
        }

        auto appearanceMenu = new MenuList(MenuItemType::Fixed, "外观",
            {new MenuItem{ListItemType::Generic, "字体", "该字体用于渲染所有UI文本", {0, 1}, font_names, 
                []() -> std::any{ return CFG_getFontId(); },
                [](const std::any &value){ CFG_setFontId(std::any_cast<int>(value)); },
                []() { CFG_setFontId(CFG_DEFAULT_FONT_ID);}},
                new MenuItem{ListItemType::Color, "主色", "用于渲染主要UI元素的颜色", colors, color_strings, 
                []() -> std::any{ return CFG_getColor(1); }, 
                [](const std::any &value){ CFG_setColor(1, std::any_cast<uint32_t>(value)); },
                []() { CFG_setColor(1, CFG_DEFAULT_COLOR1);}},
                new MenuItem{ListItemType::Color, "主要强调色", "用于突出显示用户界面中重要内容的颜色", colors, color_strings, 
                []() -> std::any{ return CFG_getColor(2); }, 
                [](const std::any &value){ CFG_setColor(2, std::any_cast<uint32_t>(value)); },
                []() { CFG_setColor(2, CFG_DEFAULT_COLOR2);}},
                new MenuItem{ListItemType::Color, "次要强调色", "次要强调色", colors, color_strings, 
                []() -> std::any{ return CFG_getColor(3); }, 
                [](const std::any &value){ CFG_setColor(3, std::any_cast<uint32_t>(value)); },
                []() { CFG_setColor(3, CFG_DEFAULT_COLOR3);}},
                new MenuItem{ListItemType::Color, "提示信息色", "按钮提示和信息的颜色", colors, color_strings, 
                []() -> std::any{ return CFG_getColor(6); }, 
                [](const std::any &value){ CFG_setColor(6, std::any_cast<uint32_t>(value)); },
                []() { CFG_setColor(6, CFG_DEFAULT_COLOR6);}},
                new MenuItem{ListItemType::Color, "列表文本", "列表文本颜色", colors, color_strings, 
                []() -> std::any{ return CFG_getColor(4); }, 
                [](const std::any &value){ CFG_setColor(4, std::any_cast<uint32_t>(value)); },
                []() { CFG_setColor(4, CFG_DEFAULT_COLOR4);}},
                new MenuItem{ListItemType::Color, "列表文本选中", "列表选中文本颜色", colors, color_strings, 
                []() -> std::any { return CFG_getColor(5); }, 
                [](const std::any &value) { CFG_setColor(5, std::any_cast<uint32_t>(value)); },
                []() { CFG_setColor(5, CFG_DEFAULT_COLOR5);}},
                new MenuItem{ListItemType::Generic, "显示电池百分比", "显示电池电量百分比", {false, true}, on_off, 
                []() -> std::any { return CFG_getShowBatteryPercent(); },
                [](const std::any &value) { CFG_setShowBatteryPercent(std::any_cast<bool>(value)); },
                []() { CFG_setShowBatteryPercent(CFG_DEFAULT_SHOWBATTERYPERCENT);}},
                new MenuItem{ListItemType::Generic, "显示菜单动画", "启用或禁用菜单动画", {false, true}, on_off, 
                []() -> std::any{ return CFG_getMenuAnimations(); },
                [](const std::any &value) { CFG_setMenuAnimations(std::any_cast<bool>(value)); },
                []() { CFG_setMenuAnimations(CFG_DEFAULT_SHOWMENUANIMATIONS);}},
                new MenuItem{ListItemType::Generic, "显示菜单过渡", "启用或禁用动画过渡", {false, true}, on_off, 
                []() -> std::any{ return CFG_getMenuTransitions(); },
                [](const std::any &value) { CFG_setMenuTransitions(std::any_cast<bool>(value)); },
                []() { CFG_setMenuTransitions(CFG_DEFAULT_SHOWMENUTRANSITIONS);}},
                new MenuItem{ListItemType::Generic, "游戏封面角落半径", "设置游戏封面的圆角半径", 0, 24, "px",
                []() -> std::any{ return CFG_getThumbnailRadius(); }, 
                [](const std::any &value) { CFG_setThumbnailRadius(std::any_cast<int>(value)); },
                []() { CFG_setThumbnailRadius(CFG_DEFAULT_THUMBRADIUS);}},
                new MenuItem{ListItemType::Generic, "游戏封面宽度", "设置游戏封面的宽度百分比。\nUI元素可能会覆盖此设置以避免裁剪", 5, 100, "%",
                []() -> std::any{ return (int)(CFG_getGameArtWidth() * 100); }, 
                [](const std::any &value) { CFG_setGameArtWidth((double)std::any_cast<int>(value) / 100.0); },
                []() { CFG_setGameArtWidth(CFG_DEFAULT_GAMEARTWIDTH);}},
                new MenuItem{ListItemType::Generic, "显示最近", "显示 \"最近播放\" 菜单项。\n这也会禁用游戏切换器", {false, true}, on_off, 
                []() -> std::any { return CFG_getShowRecents(); },
                [](const std::any &value) { CFG_setShowRecents(std::any_cast<bool>(value)); },
                []() { CFG_setShowRecents(CFG_DEFAULT_SHOWRECENTS);}},
                new MenuItem{ListItemType::Generic, "显示游戏封面", "显示游戏封面", {false, true}, on_off, []() -> std::any
                { return CFG_getShowGameArt(); },
                [](const std::any &value)
                { CFG_setShowGameArt(std::any_cast<bool>(value)); },
                []() { CFG_setShowGameArt(CFG_DEFAULT_SHOWGAMEART);}},
                new MenuItem{ListItemType::Generic, "使用文件夹背景作为ROM封面", "如果启用，使用模拟器背景图像。否则使用默认值", {false, true}, on_off, []() -> std::any
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

                new MenuItem{ListItemType::Button, "重置默认", "重置此菜单中的所有选项为默认值。", ResetCurrentMenu},
        });

        auto displayMenu = new MenuList(MenuItemType::Fixed, "显示",
        {
            new MenuItem{ListItemType::Generic, "亮度", "显示亮度 (0 到 10)", 0, 10, "",[]() -> std::any
            { return GetBrightness(); }, [](const std::any &value)
            { SetBrightness(std::any_cast<int>(value)); },
            []() { SetBrightness(SETTINGS_DEFAULT_BRIGHTNESS);}},
            new MenuItem{ListItemType::Generic, "色温", "色温 (0 到 40)", 0, 40, "",[]() -> std::any
            { return GetColortemp(); }, [](const std::any &value)
            { SetColortemp(std::any_cast<int>(value)); },
            []() { SetColortemp(SETTINGS_DEFAULT_COLORTEMP);}},
            new MenuItem{ListItemType::Generic, "对比度", "对比度增强 (-4 到 5)", -4, 5, "",[]() -> std::any
            { return GetContrast(); }, [](const std::any &value)
            { SetContrast(std::any_cast<int>(value)); },
            []() { SetContrast(SETTINGS_DEFAULT_CONTRAST);}},
            new MenuItem{ListItemType::Generic, "饱和度", "饱和度增强 (-5 到 5)", -5, 5, "",[]() -> std::any
            { return GetSaturation(); }, [](const std::any &value)
            { SetSaturation(std::any_cast<int>(value)); },
            []() { SetSaturation(SETTINGS_DEFAULT_SATURATION);}},
            new MenuItem{ListItemType::Generic, "曝光", "曝光增强 (-4 到 5)", -4, 5, "",[]() -> std::any
            { return GetExposure(); }, [](const std::any &value)
            { SetExposure(std::any_cast<int>(value)); },
            []() { SetExposure(SETTINGS_DEFAULT_EXPOSURE);}},

            new MenuItem{ListItemType::Button, "重置默认", "重置此菜单中的所有选项为默认值。", ResetCurrentMenu},
        });

        auto systemMenu = new MenuList(MenuItemType::Fixed, "系统",
        {
            new MenuItem{ListItemType::Generic, "音量", "扬声器音量",
            {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20},
            {"静音", "5%","10%","15%","20%","25%","30%","35%","40%","45%","50%","55%","60%","65%","70%","75%","80%","85%","90%","95%","100%"},
            []() -> std::any{ return GetVolume(); }, [](const std::any &value)
            { SetVolume(std::any_cast<int>(value)); },
            []() { SetVolume(SETTINGS_DEFAULT_VOLUME);}},
            new MenuItem{ListItemType::Generic, "屏幕超时", "屏幕关闭前的时间 (0-600s)", timeout_secs, timeout_labels, []() -> std::any
            { return CFG_getScreenTimeoutSecs(); }, [](const std::any &value)
            { CFG_setScreenTimeoutSecs(std::any_cast<uint32_t>(value)); },
            []() { CFG_setScreenTimeoutSecs(CFG_DEFAULT_SCREENTIMEOUTSECS);}},
            new MenuItem{ListItemType::Generic, "挂起超时", "设备进入睡眠前的时间 (0-600s)", timeout_secs, timeout_labels, []() -> std::any
            { return CFG_getSuspendTimeoutSecs(); }, [](const std::any &value)
            { CFG_setSuspendTimeoutSecs(std::any_cast<uint32_t>(value)); },
            []() { CFG_setSuspendTimeoutSecs(CFG_DEFAULT_SUSPENDTIMEOUTSECS);}},
            new MenuItem{ListItemType::Generic, "触觉反馈", "启用或禁用操作系统中某些操作的触觉反馈", {false, true}, on_off, []() -> std::any
            { return CFG_getHaptics(); }, [](const std::any &value)
            { CFG_setHaptics(std::any_cast<bool>(value)); },
            []() { CFG_setHaptics(CFG_DEFAULT_HAPTICS);}},
            new MenuItem{ListItemType::Generic, "显示24小时制", "以24小时制显示时钟", {false, true}, on_off, []() -> std::any
            { return CFG_getClock24H(); },
            [](const std::any &value)
            { CFG_setClock24H(std::any_cast<bool>(value)); },
            []() { CFG_setClock24H(CFG_DEFAULT_CLOCK24H);}},
            new MenuItem{ListItemType::Generic, "显示时钟", "在状态栏中显示时钟", {false, true}, on_off, []() -> std::any
            { return CFG_getShowClock(); },
            [](const std::any &value)
            { CFG_setShowClock(std::any_cast<bool>(value)); },
            []() { CFG_setShowClock(CFG_DEFAULT_SHOWCLOCK);}},
            new MenuItem{ListItemType::Generic, "自动设置时间和日期", "通过NTP自动调整系统时间\n(需要互联网访问)", {false, true}, on_off, []() -> std::any
            { return TIME_getNetworkTimeSync(); }, [](const std::any &value)
            { TIME_setNetworkTimeSync(std::any_cast<bool>(value)); },
            []() { TIME_setNetworkTimeSync(false);}}, // default from stock
            new MenuItem{ListItemType::Generic, "时区", "您的时区", tz_values, tz_labels, []() -> std::any
            { return std::string(TIME_getCurrentTimezone()); }, [](const std::any &value)
            { TIME_setCurrentTimezone(std::any_cast<std::string>(value).c_str()); },
            []() { TIME_setCurrentTimezone("Asia/Shanghai");}}, // default from Stock
            new MenuItem{ListItemType::Generic, "保存格式", "要使用的保存格式。\nMinUI: Game.gba.sav, Retroarch: Game.srm, Generic: Game.sav",
            {(int)SAVE_FORMAT_SAV, (int)SAVE_FORMAT_SRM, (int)SAVE_FORMAT_SRM_UNCOMPRESSED, (int)SAVE_FORMAT_GEN},
            {"MinUI (默认)", "全能模拟器（压缩）", "全能模拟器（不压缩）", "通用"}, []() -> std::any
            { return CFG_getSaveFormat(); }, [](const std::any &value)
            { CFG_setSaveFormat(std::any_cast<int>(value)); },
            []() { CFG_setSaveFormat(CFG_DEFAULT_SAVEFORMAT);}},
            new MenuItem{ListItemType::Generic, "保存状态格式", "要使用的保存状态格式。\nMinUI: Game.st0, Retroarch: Game.state.0",
            {(int)STATE_FORMAT_SAV, (int)STATE_FORMAT_SRM, (int)STATE_FORMAT_SRM_UNCOMPRESSED},
            {"MinUI (默认)", "全能模拟器（压缩）", "全能模拟器（不压缩）"}, []() -> std::any
            { return CFG_getStateFormat(); }, [](const std::any &value)
            { CFG_setStateFormat(std::any_cast<int>(value)); },
            []() { CFG_setStateFormat(CFG_DEFAULT_STATEFORMAT);}},

            new MenuItem{ListItemType::Button, "重置默认", "重置此菜单中的所有选项为默认值。", ResetCurrentMenu},
        });

        auto muteMenu = new MenuList(MenuItemType::Fixed, "FN键设置",
        {
            new MenuItem{ListItemType::Generic, "启用时调整音量", "扬声器音量 (0-20)", 
            {(int)SETTINGS_DEFAULT_MUTE_NO_CHANGE, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20}, 
            {"不变", "静音", "5%","10%","15%","20%","25%","30%","35%","40%","45%","50%","55%","60%","65%","70%","75%","80%","85%","90%","95%","100%"}, 
            []() -> std::any { return GetMutedVolume(); },
            [](const std::any &value) { SetMutedVolume(std::any_cast<int>(value)); },
            []() { SetMutedVolume(0); }},
            new MenuItem{ListItemType::Generic, "FN键禁用LED", "FN键也将禁用LED", {false, true}, on_off,
            []() -> std::any { return CFG_getMuteLEDs(); },
            [](const std::any &value) { CFG_setMuteLEDs(std::any_cast<bool>(value)); },
            []() { CFG_setMuteLEDs(CFG_DEFAULT_MUTELEDS); }},
            new MenuItem{ListItemType::Generic, "启用时调整亮度", "显示亮度 (0 到 10)", 
            {(int)SETTINGS_DEFAULT_MUTE_NO_CHANGE, 0,1,2,3,4,5,6,7,8,9,10}, 
            {"不变","0","1","2","3","4","5","6","7","8","9","10"},
            []() -> std::any { return GetMutedBrightness(); }, [](const std::any &value)
            { SetMutedBrightness(std::any_cast<int>(value)); },
            []() { SetMutedBrightness(SETTINGS_DEFAULT_MUTE_NO_CHANGE);}},
            new MenuItem{ListItemType::Generic, "启用时调整色温", "色温 (0 到 40)", 
            {(int)SETTINGS_DEFAULT_MUTE_NO_CHANGE, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40}, 
            {"不变","0","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20","21","22","23","24","25","26","27","28","29","30","31","32","33","34","35","36","37","38","39","40"},
            []() -> std::any{ return GetMutedColortemp(); }, [](const std::any &value)
            { SetMutedColortemp(std::any_cast<int>(value)); },
            []() { SetMutedColortemp(SETTINGS_DEFAULT_MUTE_NO_CHANGE);}},
            new MenuItem{ListItemType::Generic, "启用时调整对比度", "对比度增强 (-4 到 5)", 
            {(int)SETTINGS_DEFAULT_MUTE_NO_CHANGE, -4,-3,-2,-1,0,1,2,3,4,5}, 
            {"不变","-4","-3","-2","-1","0","1","2","3","4","5"}, 
            []() -> std::any  { return GetMutedContrast(); }, [](const std::any &value)
            { SetMutedContrast(std::any_cast<int>(value)); },
            []() { SetMutedContrast(SETTINGS_DEFAULT_MUTE_NO_CHANGE);}},
            new MenuItem{ListItemType::Generic, "启用时调整饱和度", "饱和度增强 (-5 到 5)", 
            {(int)SETTINGS_DEFAULT_MUTE_NO_CHANGE, -5,-4,-3,-2,-1,0,1,2,3,4,5}, 
            {"不变","-5","-4","-3","-2","-1","0","1","2","3","4","5"}, 
            []() -> std::any{ return GetMutedSaturation(); }, [](const std::any &value)
            { SetMutedSaturation(std::any_cast<int>(value)); },
            []() { SetMutedSaturation(SETTINGS_DEFAULT_MUTE_NO_CHANGE);}},
            new MenuItem{ListItemType::Generic, "启用时调整曝光", "曝光增强 (-4 到 5)", 
            {(int)SETTINGS_DEFAULT_MUTE_NO_CHANGE, -4,-3,-2,-1,0,1,2,3,4,5}, 
            {"不变","-4","-3","-2","-1","0","1","2","3","4","5"}, 
            []() -> std::any  { return GetMutedExposure(); }, [](const std::any &value)
            { SetMutedExposure(std::any_cast<int>(value)); },
            []() { SetMutedExposure(SETTINGS_DEFAULT_MUTE_NO_CHANGE);}},

            new MenuItem{ListItemType::Button, "重置默认", "重置此菜单中的所有选项为默认值。", ResetCurrentMenu},
        });

        // TODO: check WIFI_supported(), hide menu otherwise
        auto networkMenu = new Wifi::Menu(appQuit);

        ctx.menu = new MenuList(MenuItemType::List, "主菜单",
        {
            new MenuItem{ListItemType::Generic, "外观", "UI自定义", {}, {}, nullptr, nullptr, DeferToSubmenu, appearanceMenu},
            new MenuItem{ListItemType::Generic, "显示", "", {}, {}, nullptr, nullptr, DeferToSubmenu, displayMenu},
            new MenuItem{ListItemType::Generic, "系统", "", {}, {}, nullptr, nullptr, DeferToSubmenu, systemMenu},
            new MenuItem{ListItemType::Generic, "FN键", "FN键设置", {}, {}, nullptr, nullptr, DeferToSubmenu, muteMenu},
            new MenuItem{ListItemType::Generic, "网络", "", {}, {}, nullptr, nullptr, DeferToSubmenu, networkMenu},
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
                        char *hints[] = {(char *)("菜单"), (char *)("睡眠"), NULL};
                        GFX_blitButtonGroup(hints, 0, ctx.screen, 0);
                    }
                    char *hints[] = {(char *)("B"), (char *)("返回"), (char *)("A"), (char *)("确定"), NULL};
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
