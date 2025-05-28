#include "i18n.h"
#include "config.h"
#include "api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Language current_language = LANG_EN_US; // Default to English, will be read from settings
static bool initialized = false;

// Language change callback management
#define MAX_CALLBACKS 10
static LanguageChangeCallback callbacks[MAX_CALLBACKS];
static int callback_count = 0;

// Language name mapping
static const char* language_names[LANG_COUNT] = {
    "English",
    "简体中文", 
    "日本語",
    "한국어"
};

// Translation table structure
typedef struct {
    const char* key;
    const char* translations[LANG_COUNT];
} TranslationEntry;

// Main translation table
static TranslationEntry translation_table[] = {
    // Button labels
    {"A", {"A", "A", "A", "A"}},
    {"B", {"B", "B", "B", "B"}},
    {"X", {"X", "X", "X", "X"}},
    {"Y", {"Y", "Y", "Y", "Y"}},
    {"L1", {"L1", "L1", "L1", "L1"}},
    {"R1", {"R1", "R1", "R1", "R1"}},
    {"L2", {"L2", "L2", "L2", "L2"}},
    {"R2", {"R2", "R2", "R2", "R2"}},
    {"L3", {"L3", "L3", "L3", "L3"}},
    {"R3", {"R3", "R3", "R3", "R3"}},
    {"UP", {"Up", "上", "上へ", "위로"}},
    {"DOWN", {"Down", "下", "下へ", "아래로"}},
    {"LEFT", {"Left", "左", "左へ", "왼쪽으로"}},
    {"RIGHT", {"Right", "右", "右へ", "오른쪽으로"}},
    {"SELECT", {"Select", "选择", "選択", "선택"}},
    {"START", {"Start", "开始", "スタート", "시작"}},
    {"HOME", {"Home", "主页", "ホーム", "홈"}},
    {"POWER", {"Power", "电源", "電源", "전원"}},
    {"MENU", {"Menu", "菜单", "メニュー", "메뉴"}},
    {"VOLUME_UP", {"+", "+", "+", "+"}},
    {"VOLUME_DOWN", {"-", "-", "-", "-"}},

    // Common Actions
    {"confirm", {"Confirm", "确认", "確認", "확인"}},
    {"ok", {"OK", "确定", "OK", "확인"}},
    {"yes", {"Yes", "是", "はい", "예"}},
    {"no", {"No", "否", "いいえ", "아니오"}},
    {"apply", {"Apply", "应用", "適用", "적용"}},
    {"cancel", {"Cancel", "取消", "キャンセル", "취소"}},
    {"reset", {"Reset", "重置", "リセット", "재설정"}},
    {"back", {"Back", "返回", "戻る", "뒤로"}},
    {"sleep", {"Sleep", "待机", "スリープ", "절전"}},
    {"empty_folder", {"Empty folder", "空文件夹", "空のフォルダ", "빈 폴더"}},
    

    // Settings menu - using exact strings from original code
    {"main", {"Main", "主菜单", "メイン", "메인"}},
    {"appearance", {"Appearance", "外观", "外観", "외관"}},
    {"ui_customization", {"UI customization", "UI自定义", "UIカスタマイズ", "UI 커스터마이제이션"}},
    {"display", {"Display", "显示", "ディスプレイ", "디스플레이"}},
    {"system", {"System", "系统", "システム", "시스템"}},
    {"network", {"Network", "网络", "ネットワーク", "네트워크"}},
    {"fn_switch", {"FN switch", "FN开关", "FNスイッチ", "FN 스위치"}},
    {"fn_switch_settings", {"FN switch settings", "FN开关设置", "FNスイッチ設定", "FN 스위치 설정"}},
    
    // Appearance menu items - using exact strings from original code
    {"font", {"Font", "字体", "フォント", "폰트"}},
    {"the_font_to_render_all_ui_text", {"The font to render all UI text.", "该字体用于渲染所有UI文本。", "すべてのUIテキストのレンダリングに使用するフォント。", "모든 UI 텍스트 렌더링에 사용할 폰트입니다."}},
    {"main_color", {"Main Color", "主色", "メインカラー", "메인 컬러"}},
    {"the_color_used_to_render_main_ui_elements", {"The color used to render main UI elements.", "用于渲染主要UI元素的颜色。", "メインUI要素のレンダリングに使用する色。", "메인 UI 요소 렌더링에 사용되는 색상입니다."}},
    {"primary_accent_color", {"Primary Accent Color", "主要强调色", "プライマリアクセントカラー", "기본 강조 색상"}},
    {"the_color_used_to_highlight_important_things_in_the_user_interface", {"The color used to highlight important things in the user interface.", "用于突出显示用户界面中重要内容的颜色。", "ユーザーインターフェースの重要な要素を強調表示するために使用される色。", "사용자 인터페이스에서 중요한 요소를 강조 표시하는 데 사용되는 색상입니다."}},
    {"secondary_accent_color", {"Secondary Accent Color", "次要强调色", "セカンダリアクセントカラー", "보조 강조 색상"}},
    {"a_secondary_highlight_color", {"A secondary highlight color.", "次要强调色。", "セカンダリハイライトカラー。", "보조 강조 색상입니다."}},
    {"hint_info_color", {"Hint info Color", "提示信息色", "ヒント情報カラー", "힌트 정보 색상"}},
    {"color_for_button_hints_and_info", {"Color for button hints and info", "按钮提示和信息的颜色", "ボタンのヒントと情報の色", "버튼 힌트 및 정보 색상"}},
    {"list_text", {"List Text", "列表文本", "リストテキスト", "목록 텍스트"}},
    {"list_text_color", {"List text color", "列表文本颜色", "リストテキストの色", "목록 텍스트 색상"}},
    {"list_text_selected", {"List Text Selected", "列表文本选中", "リストテキスト選択", "목록 텍스트 선택됨"}},
    {"list_selected_text_color", {"List selected text color", "列表选中文本颜色", "選択されたリストテキストの色", "선택된 목록 텍스트 색상"}},
    {"show_battery_percentage", {"Show battery percentage", "显示电池百分比", "バッテリー残量をパーセンテージで表示", "배터리 백분율 표시"}},
    {"show_battery_level_as_percent_in_the_status_pill", {"Show battery level as percent in the status pill", "在状态栏中以百分比显示电池电量", "ステータスピルでバッテリーレベルをパーセンテージで表示", "상태 표시줄에서 배터리 레벨을 백분율로 표시"}},
    {"show_menu_animations", {"Show menu animations", "显示菜单动画", "メニューアニメーションを表示", "메뉴 애니메이션 표시"}},
    {"enable_or_disable_menu_animations", {"Enable or disable menu animations", "启用或禁用菜单动画", "メニューアニメーションを有効または無効にする", "메뉴 애니메이션 활성화 또는 비활성화"}},
    {"show_menu_transitions", {"Show menu transitions", "显示菜单过渡", "メニュー遷移を表示", "메뉴 전환 효과 표시"}},
    {"enable_or_disable_animated_transitions", {"Enable or disable animated transitions", "启用或禁用动画过渡", "アニメーション遷移を有効または無効にする", "애니메이션 전환 효과 활성화 또는 비활성화"}},
    {"game_art_corner_radius", {"Game art corner radius", "游戏封面角落半径", "ゲームアートの角の半径", "게임 아트 모서리 반지름"}},
    {"set_the_radius_for_the_rounded_corners_of_game_art", {"Set the radius for the rounded corners of game art", "设置游戏封面圆角的半径", "ゲームアートの角丸の半径を設定", "게임 아트의 둥근 모서리 반지름 설정"}},
    {"game_art_width", {"Game art width", "游戏封面宽度", "ゲームアートの幅", "게임 아트 너비"}},
    {"set_the_percentage_of_screen_width_used_for_game_art", {"Set the percentage of screen width used for game art.\nUI elements might overrule this to avoid clipping.", "设置游戏封面所占屏幕宽度的百分比。\nUI元素可能会覆盖此设置以避免裁剪。", "ゲームアートに使用する画面幅のパーセンテージを設定。\nUI要素がクリッピングを避けるためにこの設定を上書きする場合があります。", "게임 아트에 사용되는 화면 너비의 백분율을 설정합니다.\nUI 요소가 잘림을 방지하기 위해 이 설정을 재정의할 수 있습니다."}},
    {"show_recents", {"Show recents", "显示最近", "最近項目を表示", "최근 항목 표시"}},
    {"show_recently_played_menu_entry", {"Show \"Recently Played\" menu entry.\nThis also disables Game Switcher.", "显示\"最近游玩\"菜单项。\n这也会禁用游戏切换器。", "「最近プレイした」メニュー項目を表示。\nこれによりゲームスイッチャーも無効になります。", "\"최근 플레이한\" 메뉴 항목을 표시합니다.\n이렇게 하면 게임 스위처도 비활성화됩니다."}},
    {"show_game_art", {"Show game art", "显示游戏封面", "ゲームアートを表示", "게임 아트 표시"}},
    {"show_game_artwork_in_the_main_menu", {"Show game artwork in the main menu", "在主菜单中显示游戏封面", "メインメニューでゲームアートワークを表示", "메인 메뉴에서 게임 아트워크 표시"}},
    {"use_folder_background_for_roms", {"Use folder background for ROMs", "使用文件夹背景作为ROM封面", "ROMにフォルダ背景を使用", "ROM에 폴더 배경 사용"}},
    {"if_enabled_used_the_emulator_background_image", {"If enabled, used the emulator background image. Otherwise uses the default.", "如果启用，使用模拟器背景图像。否则使用默认值。", "有効にした場合、エミュレータの背景画像を使用。そうでなければデフォルトを使用。", "활성화된 경우 에뮬레이터 배경 이미지를 사용합니다. 그렇지 않으면 기본값을 사용합니다."}},
    {"reset_to_defaults", {"Reset to defaults", "重置为默认值", "デフォルトにリセット", "기본값으로 재설정"}},
    {"resets_all_options_in_this_menu_to_their_default_values", {"Resets all options in this menu to their default values.", "将此菜单中的所有选项重置为默认值。", "このメニューのすべてのオプションをデフォルト値にリセットします。", "이 메뉴의 모든 옵션을 기본값으로 재설정합니다."}},
    
    // Display menu items - using exact strings from original code
    {"brightness", {"Brightness", "亮度", "明るさ", "밝기"}},
    {"display_brightness_0_to_10", {"Display brightness (0 to 10)", "显示亮度 (0 到 10)", "ディスプレイの明るさ（0から10）", "디스플레이 밝기 (0에서 10)"}},
    {"color_temperature", {"Color temperature", "色温", "色温度", "색온도"}},
    {"color_temperature_0_to_40", {"Color temperature (0 to 40)", "色温 (0 到 40)", "色温度（0から40）", "색온도 (0에서 40)"}},
    {"contrast", {"Contrast", "对比度", "コントラスト", "대비"}},
    {"contrast_enhancement_minus4_to_5", {"Contrast enhancement (-4 to 5)", "对比度增强 (-4 到 5)", "コントラスト強化（-4から5）", "대비 강화 (-4에서 5)"}},
    {"saturation", {"Saturation", "饱和度", "彩度", "채도"}},
    {"saturation_enhancement_minus5_to_5", {"Saturation enhancement (-5 to 5)", "饱和度增强 (-5 到 5)", "彩度強化（-5から5）", "채도 강화 (-5에서 5)"}},
    {"exposure", {"Exposure", "曝光", "露出", "노출"}},
    {"exposure_enhancement_minus4_to_5", {"Exposure enhancement (-4 to 5)", "曝光增强 (-4 到 5)", "露出強化（-4から5）", "노출 강화 (-4에서 5)"}},
    
    // System menu items - using exact strings from original code
    {"volume", {"Volume", "音量", "音量", "볼륨"}},
    {"speaker_volume", {"Speaker volume", "扬声器音量", "スピーカー音量", "스피커 볼륨"}},
    {"screen_timeout", {"Screen timeout", "屏幕超时", "画面タイムアウト", "화면 시간 초과"}},
    {"time_before_screen_turns_off_0_600s", {"Time before screen turns off (0-600s)", "屏幕关闭前的时间 (0-600s)", "画面がオフになるまでの時間（0-600秒）", "화면이 꺼지기까지의 시간 (0-600초)"}},
    {"suspend_timeout", {"Suspend timeout", "挂起超时", "サスペンドタイムアウト", "절전 시간 초과"}},
    {"time_before_device_goes_to_sleep_0_600s", {"Time before device goes to sleep (0-600s)", "设备进入睡眠前的时间 (0-600s)", "デバイスがスリープに入るまでの時間（0-600秒）", "장치가 절전 모드로 들어가기까지의 시간 (0-600초)"}},
    {"haptic_feedback", {"Haptic feedback", "触觉反馈", "ハプティックフィードバック", "햅틱 피드백"}},
    {"enable_or_disable_haptic_feedback_on_certain_actions_in_the_os", {"Enable or disable haptic feedback on certain actions in the OS", "启用或禁用操作系统中某些操作的触觉反馈", "OSの特定の操作に対するハプティックフィードバックを有効または無効にする", "OS의 특정 작업에 대한 햅틱 피드백 활성화 또는 비활성화"}},
    {"show_24h_time_format", {"Show 24h time format", "显示24小时制", "24時間形式で表示", "24시간 형식 표시"}},
    {"show_clock_in_the_24hrs_time_format", {"Show clock in the 24hrs time format", "以24小时制显示时钟", "24時間形式で時計を表示", "24시간 형식으로 시계 표시"}},
    {"show_clock", {"Show clock", "显示时钟", "時計を表示", "시계 표시"}},
    {"show_clock_in_the_status_pill", {"Show clock in the status pill", "在状态栏中显示时钟", "ステータスピルに時計を表示", "상태 표시줄에 시계 표시"}},
    {"set_time_and_date_automatically", {"Set time and date automatically", "自动设置时间和日期", "時刻と日付を自動設定", "시간 및 날짜 자동 설정"}},
    {"automatically_adjust_system_time_with_ntp", {"Automatically adjust system time\nwith NTP (requires internet access)", "通过NTP自动调整系统时间\n（需要互联网访问）", "NTPによるシステム時刻の自動調整\n（インターネットアクセスが必要）", "NTP를 통해 시스템 시간을 자동으로 조정\n(인터넷 연결 필요)"}},    // Font names
    {"font_1", {"Next", "寒蝉半圆体", "Next", "Next"}},
    {"font_2", {"OG", "寒蝉全圆体", "OG", "OG"}},
    
    // Additional strings for settings - timeout labels
    {"never", {"Never", "从不", "なし", "없음"}},
    {"5_seconds", {"5 seconds", "5秒", "5秒", "5초"}},
    {"10_seconds", {"10 seconds", "10秒", "10秒", "10초"}},
    {"15_seconds", {"15 seconds", "15秒", "15秒", "15초"}},
    {"30_seconds", {"30 seconds", "30秒", "30秒", "30초"}},
    {"45_seconds", {"45 seconds", "45秒", "45秒", "45초"}},
    {"60_seconds", {"60 seconds", "60秒", "60秒", "60초"}},
    {"90_seconds", {"90 seconds", "90秒", "90秒", "90초"}},
    {"2_minutes", {"2 minutes", "2分钟", "2分", "2분"}},
    {"4_minutes", {"4 minutes", "4分钟", "4分", "4분"}},
    {"6_minutes", {"6 minutes", "6分钟", "6分", "6분"}},
    {"10_minutes", {"10 minutes", "10分钟", "10分", "10분"}},
    
    // Volume labels
    {"mute", {"Mute", "静音", "ミュート", "음소거"}},
    {"5_percent", {"5%", "5%", "5%", "5%"}},
    {"10_percent", {"10%", "10%", "10%", "10%"}},
    {"15_percent", {"15%", "15%", "15%", "15%"}},
    {"20_percent", {"20%", "20%", "20%", "20%"}},
    {"25_percent", {"25%", "25%", "25%", "25%"}},
    {"30_percent", {"30%", "30%", "30%", "30%"}},
    {"35_percent", {"35%", "35%", "35%", "35%"}},
    {"40_percent", {"40%", "40%", "40%", "40%"}},
    {"45_percent", {"45%", "45%", "45%", "45%"}},
    {"50_percent", {"50%", "50%", "50%", "50%"}},
    {"55_percent", {"55%", "55%", "55%", "55%"}},
    {"60_percent", {"60%", "60%", "60%", "60%"}},
    {"65_percent", {"65%", "65%", "65%", "65%"}},
    {"70_percent", {"70%", "70%", "70%", "70%"}},
    {"75_percent", {"75%", "75%", "75%", "75%"}},
    {"80_percent", {"80%", "80%", "80%", "80%"}},
    {"85_percent", {"85%", "85%", "85%", "85%"}},
    {"90_percent", {"90%", "90%", "90%", "90%"}},
    {"95_percent", {"95%", "95%", "95%", "95%"}},
    {"100_percent", {"100%", "100%", "100%", "100%"}},
    
    // Scaling options
    {"fullscreen", {"Fullscreen", "全屏", "フルスクリーン", "전체 화면"}},
    {"fit", {"Fit", "适应", "フィット", "맞춤"}},
    {"fill", {"Fill", "填充", "フィル", "채우기"}},
    
    // Save format options
    {"minui_default", {"MinUI (default)", "MinUI (默认)", "MinUI（デフォルト）", "MinUI (기본값)"}},
    {"retroarch_compressed", {"Retroarch (compressed)", "全能模拟器（压缩）", "Retroarch（圧縮）", "Retroarch (압축)"}},
    {"retroarch_uncompressed", {"Retroarch (uncompressed)", "全能模拟器（不压缩）", "Retroarch（非圧縮）", "Retroarch (비압축)"}},
    {"generic", {"Generic", "通用", "ジェネリック", "일반"}},
    // Additional strings
    {"language_setting", {"Language", "语言", "言語", "언어"}},
    {"interface_language_setting", {"Interface language setting", "界面语言设置", "インターフェース言語設定", "인터페이스 언어 설정"}},
    {"timezone", {"Timezone", "时区", "タイムゾーン", "시간대"}},
    {"your_timezone", {"Your timezone", "您的时区", "あなたのタイムゾーン", "귀하의 시간대"}},
    {"save_format", {"Save format", "保存格式", "セーブ形式", "저장 형식"}},
    {"save_format_description", {"Save format to use.\nMinUI: Game.gba.sav, Retroarch: Game.srm, Generic: Game.sav", "要使用的保存格式。\nMinUI: Game.gba.sav, Retroarch: Game.srm, Generic: Game.sav", "使用するセーブ形式。\nMinUI: Game.gba.sav, Retroarch: Game.srm, Generic: Game.sav", "사용할 저장 형식.\nMinUI: Game.gba.sav, Retroarch: Game.srm, Generic: Game.sav"}},
    {"save_state_format", {"Save state format", "保存状态格式", "セーブステート形式", "저장 상태 형식"}},
    {"save_state_format_description", {"Save state format to use.\nMinUI: Game.st0, Retroarch: Game.state.0", "要使用的保存状态格式。\nMinUI: Game.st0, Retroarch: Game.state.0", "使用するセーブステート形式。\nMinUI: Game.st0, Retroarch: Game.state.0", "사용할 저장 상태 형식.\nMinUI: Game.st0, Retroarch: Game.state.0"}},
    
    // FN menu strings
    {"adjust_volume_when_enabled", {"Adjust volume when enabled", "启用时调整音量", "有効時の音量調整", "활성화 시 볼륨 조정"}},
    {"speaker_volume_0_20", {"Speaker volume (0-20)", "扬声器音量 (0-20)", "スピーカー音量（0-20）", "스피커 볼륨 (0-20)"}},
    {"fn_key_disable_leds", {"FN key disable LEDs", "FN键禁用LED", "FNキーでLED無効", "FN 키로 LED 비활성화"}},
    {"fn_key_will_also_disable_leds", {"FN key will also disable LEDs", "FN键也将禁用LED", "FNキーはLEDも無効にします", "FN 키는 LED도 비활성화합니다"}},
    {"adjust_brightness_when_enabled", {"Adjust brightness when enabled", "启用时调整亮度", "有効時の明るさ調整", "활성화 시 밝기 조정"}},
    {"adjust_color_temperature_when_enabled", {"Adjust color temperature when enabled", "启用时调整色温", "有効時の色温度調整", "활성화 시 색온도 조정"}},
    {"adjust_contrast_when_enabled", {"Adjust contrast when enabled", "启用时调整对比度", "有効時のコントラスト調整", "활성화 시 대비 조정"}},
    {"adjust_saturation_when_enabled", {"Adjust saturation when enabled", "启用时调整饱和度", "有効時の彩度調整", "활성화 시 채도 조정"}},
    {"adjust_exposure_when_enabled", {"Adjust exposure when enabled", "启用时调整曝光", "有効時の露出調整", "활성화 시 노출 조정"}},
    {"no_change", {"No change", "不变", "変更なし", "변경 없음"}}

    // Wifi Menu
    {"wifi_network_title", {"Network", "网络", "ネットワーク", "네트워크"}},
    {"wifi_toggle_label", {"WiFi", "WiFi", "WiFi", "WiFi"}},
    {"wifi_toggle_desc", {"Enable/disable WiFi", "启用/禁用 WiFi", "WiFiを有効/無効にする", "WiFi 활성화/비활성화"}},
    {"wifi_off", {"Off", "关闭", "オフ", "꺼짐"}},
    {"wifi_on", {"On", "开启", "オン", "켜짐"}},
    {"wifi_connect", {"Connect", "连接", "接続", "연결"}},
    {"wifi_connect_desc", {"Connect to this network.", "连接到此网络。", "このネットワークに接続します。", "이 네트워크에 연결합니다."}},
    {"wifi_enter_password", {"Enter WiFi Password", "输入 WiFi 密码", "WiFiパスワードを入力", "WiFi 비밀번호 입력"}},
    {"wifi_forget", {"Forget", "忘记", "削除", "잊음"}},
    {"wifi_forget_desc", {"Delete credentials for this network.", "删除此网络的凭据。", "このネットワークの資格情報を削除します。", "이 네트워크의 자격 증명을 삭제합니다."}},
    {"wifi_disconnect", {"Disconnect", "断开连接", "切断", "연결 끊기"}},
    {"wifi_disconnect_desc", {"Disconnect from this network.", "从此网络断开连接。", "このネットワークから切断します。", "이 네트워크에서 연결을 끊습니다."}},
    {"wifi_options", {"Options", "选项", "オプション", "옵션"}}

    // api.c    {"brightness_api", {"BRIGHTNESS", "亮度", "明るさ", "밝기"}},
    {"color_temperature_api", {"COLOR TEMP", "色温", "色温度", "색온도"}},
    {"menu_api", {"MNU", "菜单", "メニュー", "메뉴"}},
    {"select_api", {"SEL", "选择", "選択", "선택"}},
    {"BRIGHT", {"BRIGHT", "亮度", "明るさ", "밝기"}},
    {"CLTEMP", {"CLTEMP", "色温", "色温度", "색온도"}},
    {"quicksave_created_powering_off",{"Quicksave created,\npowering off", "已创建快速存档\n现在关机", "クイックセーブが作成されました\n電源を切ります", "퀵세이브가 생성되었습니다\n전원을 끕니다"}},
    {"powering_off", {"Powering off", "正在关机", "電源を切っています", "전원을 끕니다"}}

};

void I18N_init(void) {
    if (initialized) return;
    
    // Get language from config
    current_language = (Language)CFG_getLanguage();
    if (current_language < 0 || current_language >= LANG_COUNT) {
        current_language = LANG_EN_US;
    }
    
    initialized = true;
    printf("I18N initialized with language: %s\n", language_names[current_language]);
}

const char* I18N_get(const char* key) {
    if (!initialized) {
        I18N_init();
    }
    
    for (int i = 0; i < sizeof(translation_table) / sizeof(TranslationEntry); ++i) {
        if (strcmp(translation_table[i].key, key) == 0) {
            return translation_table[i].translations[current_language];
        }
    }
    
    return key; // Return the key itself if no translation is found
}

void I18N_setLanguage(Language lang) {
    if (lang < 0 || lang >= LANG_COUNT) {
        return;
    }
    
    current_language = lang;
    
    // Notify all registered callbacks
    for (int i = 0; i < callback_count; ++i) {
        if (callbacks[i]) {
            callbacks[i]();
        }
    }
}

Language I18N_getCurrentLanguage(void) {
    return current_language;
}

const char* I18N_getLanguageName(Language lang) {
    if (lang < 0 || lang >= LANG_COUNT) {
        return language_names[LANG_EN_US];
    }
    return language_names[lang];
}

void I18N_registerCallback(LanguageChangeCallback callback) {
    if (callback_count < MAX_CALLBACKS && callback) {
        callbacks[callback_count++] = callback;
    }
}

void I18N_unregisterCallback(LanguageChangeCallback callback) {
    for (int i = 0; i < callback_count; ++i) {
        if (callbacks[i] == callback) {
            // Shift remaining callbacks down
            for (int j = i; j < callback_count - 1; ++j) {
                callbacks[j] = callbacks[j + 1];
            }
            callback_count--;
            break;
        }
    }
}

void I18N_cleanup(void) {
    callback_count = 0;
    initialized = false;
}
