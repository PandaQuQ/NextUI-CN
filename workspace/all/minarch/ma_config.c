#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <dirent.h>

#include "ma_internal.h"
#include "ma_options.h"
#include "ma_config.h"

static ButtonMapping button_label_mapping[] = { // used to lookup the retro_id and local btn_id from button name
	{"NONE",	-1,								BTN_ID_NONE},
	{"UP",		RETRO_DEVICE_ID_JOYPAD_UP,		BTN_ID_DPAD_UP},
	{"DOWN",	RETRO_DEVICE_ID_JOYPAD_DOWN,	BTN_ID_DPAD_DOWN},
	{"LEFT",	RETRO_DEVICE_ID_JOYPAD_LEFT,	BTN_ID_DPAD_LEFT},
	{"RIGHT",	RETRO_DEVICE_ID_JOYPAD_RIGHT,	BTN_ID_DPAD_RIGHT},
	{"A",		RETRO_DEVICE_ID_JOYPAD_A,		BTN_ID_A},
	{"B",		RETRO_DEVICE_ID_JOYPAD_B,		BTN_ID_B},
	{"X",		RETRO_DEVICE_ID_JOYPAD_X,		BTN_ID_X},
	{"Y",		RETRO_DEVICE_ID_JOYPAD_Y,		BTN_ID_Y},
	{"START",	RETRO_DEVICE_ID_JOYPAD_START,	BTN_ID_START},
	{"SELECT",	RETRO_DEVICE_ID_JOYPAD_SELECT,	BTN_ID_SELECT},
	{"L1",		RETRO_DEVICE_ID_JOYPAD_L,		BTN_ID_L1},
	{"R1",		RETRO_DEVICE_ID_JOYPAD_R,		BTN_ID_R1},
	{"L2",		RETRO_DEVICE_ID_JOYPAD_L2,		BTN_ID_L2},
	{"R2",		RETRO_DEVICE_ID_JOYPAD_R2,		BTN_ID_R2},
	{"L3",		RETRO_DEVICE_ID_JOYPAD_L3,		BTN_ID_L3},
	{"R3",		RETRO_DEVICE_ID_JOYPAD_R3,		BTN_ID_R3},
	{NULL,0,0}
};

static int Config_getValue(char* cfg, const char* key, char* out_value, int* lock) { // gets value from string
	char* tmp = cfg;
	while ((tmp = strstr(tmp, key))) {
		if (lock!=NULL && tmp>cfg && *(tmp-1)=='-') *lock = 1; // prefixed with a `-` means lock
		tmp += strlen(key);
		if (!strncmp(tmp, " = ", 3)) break; // matched
	};
	if (!tmp) return 0;
	tmp += 3;
	
	strncpy(out_value, tmp, 256);
	out_value[256 - 1] = '\0';
	tmp = strchr(out_value, '\n');
	if (!tmp) tmp = strchr(out_value, '\r');
	if (tmp) *tmp = '\0';

	// LOG_info("\t%s = %s (%s)\n", key, out_value, (lock && *lock) ? "hidden":"shown");
	return 1;
}

void updateCPUMonitor(void) {
    Perf_setCPUMonitorEnabled(show_debug);
    if (!show_debug) return;

    pthread_t cpucheckthread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (pthread_create(&cpucheckthread, &attr, PLAT_cpu_monitor, NULL) != 0) {
        LOG_info("WARNING: failed to start CPU monitor thread\n");
        Perf_setCPUMonitorEnabled(0);
    }
    pthread_attr_destroy(&attr);
}

void setOverclock(int i) {
	overclock = i;
	PWR_setCPUSpeed(i);
}
void Config_syncFrontend(char* key, int value) {
	int i = -1;
	if (exactMatch(key,config.frontend.options[FE_OPT_SCALING].key)) {
		screen_scaling 	= value;
		
		renderer.dst_p = 0;
		i = FE_OPT_SCALING;
	}
	else if (exactMatch(key,config.frontend.options[FE_OPT_RESAMPLING].key)) {
		resampling_quality = value;
		SND_setQuality(resampling_quality);
		i = FE_OPT_RESAMPLING;
	}
	else if (exactMatch(key,config.frontend.options[FE_OPT_AMBIENT].key)) {
		ambient_mode = value;
		if(ambient_mode > 0)
			LEDS_pushProfileOverride(LIGHT_PROFILE_AMBIENT);
		else 
			LEDS_popProfileOverride(LIGHT_PROFILE_AMBIENT);
		i = FE_OPT_AMBIENT;
	}
	else if (exactMatch(key,config.frontend.options[FE_OPT_EFFECT].key)) {
		screen_effect = value;
		GFX_setEffect(value);
		renderer.dst_p = 0;
		i = FE_OPT_EFFECT;
	}
	else if (exactMatch(key,config.frontend.options[FE_OPT_OVERLAY].key)) {
		char** overlayList = config.frontend.options[FE_OPT_OVERLAY].values;
		if (overlayList) {
			
			int count = 0;
			while (overlayList && overlayList[count]) count++;
			if (value >= 0 && value < count) {
				LOG_info("minarch: updating overlay - %s\n", overlayList[value]);
				GFX_setOverlay(overlayList[value], core.tag);
				overlay = value;
				renderer.dst_p = 0;
				i = FE_OPT_OVERLAY;
			}
		}
	}
	else if (exactMatch(key,config.frontend.options[FE_OPT_SCREENX].key)) {
		cfg_screenx = value;
		GFX_setOffsetX(value);
		i = FE_OPT_SCREENX;
	}
	else if (exactMatch(key,config.frontend.options[FE_OPT_SCREENY].key)) {
		cfg_screeny = value;
		GFX_setOffsetY(value);
		i = FE_OPT_SCREENY;
	}
	else if (exactMatch(key,config.frontend.options[FE_OPT_SHARPNESS].key)) {
		GFX_setSharpness(value);
		i = FE_OPT_SHARPNESS;
	}
	else if (exactMatch(key,config.frontend.options[FE_OPT_SYNC_REFERENCE].key)) {
		sync_ref = value;
		i = FE_OPT_SYNC_REFERENCE;
	}
	else if (exactMatch(key,config.frontend.options[FE_OPT_OVERCLOCK].key)) {
		overclock = value;
		i = FE_OPT_OVERCLOCK;
	}
	else if (exactMatch(key,config.frontend.options[FE_OPT_DEBUG].key)) {
        int prev_show_debug = show_debug;
        show_debug = value;
        if (prev_show_debug != show_debug) updateCPUMonitor();
		i = FE_OPT_DEBUG;
	}
	else if (exactMatch(key,config.frontend.options[FE_OPT_MAXFF].key)) {
		max_ff_speed = value;
		i = FE_OPT_MAXFF;
	}
	else if (exactMatch(key,config.frontend.options[FE_OPT_FF_AUDIO].key)) {
		ff_audio = value;
		i = FE_OPT_FF_AUDIO;
	}
	else if (exactMatch(key,config.frontend.options[FE_OPT_REWIND_ENABLE].key)) {
		i = FE_OPT_REWIND_ENABLE;
	}
	else if (exactMatch(key,config.frontend.options[FE_OPT_REWIND_BUFFER].key)) {
		i = FE_OPT_REWIND_BUFFER;
	}
	else if (exactMatch(key,config.frontend.options[FE_OPT_REWIND_GRANULARITY].key)) {
		i = FE_OPT_REWIND_GRANULARITY;
	}
	else if (exactMatch(key,config.frontend.options[FE_OPT_REWIND_AUDIO].key)) {
		i = FE_OPT_REWIND_AUDIO;
	}
	else if (exactMatch(key,config.frontend.options[FE_OPT_REWIND_COMPRESSION].key)) {
		i = FE_OPT_REWIND_COMPRESSION;
	}
	else if (exactMatch(key,config.frontend.options[FE_OPT_REWIND_COMPRESSION_ACCEL].key)) {
		i = FE_OPT_REWIND_COMPRESSION_ACCEL;
	}
	if (i==-1) return;
	Option* option = &config.frontend.options[i];
	option->value = value;
	if (i==FE_OPT_REWIND_ENABLE || i==FE_OPT_REWIND_BUFFER || i==FE_OPT_REWIND_GRANULARITY || i==FE_OPT_REWIND_AUDIO || i==FE_OPT_REWIND_COMPRESSION || i==FE_OPT_REWIND_COMPRESSION_ACCEL) {
		const char* sval = option->values && option->values[value] ? option->values[value] : "0";
		int parsed = 0;
		if (i==FE_OPT_REWIND_ENABLE || i==FE_OPT_REWIND_AUDIO || i==FE_OPT_REWIND_COMPRESSION) {
			// use option index (Off/On)
			parsed = value;
		}
		else {
			parsed = strtol(sval, NULL, 10);
		}
		switch (i) {
			case FE_OPT_REWIND_ENABLE: rewind_cfg_enable = parsed; break;
			case FE_OPT_REWIND_BUFFER: rewind_cfg_buffer_mb = parsed; break;
			case FE_OPT_REWIND_GRANULARITY: rewind_cfg_granularity = parsed; break;
			case FE_OPT_REWIND_AUDIO: rewind_cfg_audio = parsed; break;
			case FE_OPT_REWIND_COMPRESSION: rewind_cfg_compress = parsed; break;
			case FE_OPT_REWIND_COMPRESSION_ACCEL: rewind_cfg_lz4_acceleration = parsed; break;
		}
		// Defer Rewind_init until after the explicit init in main(), otherwise FBN crashes
		if (rewind_init_ready) {
			Rewind_init(core.serialize_size ? core.serialize_size() : 0);
		}
		if (i==FE_OPT_REWIND_ENABLE) {
			// ensure runtime toggles don't linger when enabling/disabling feature
			rewind_toggle = 0;
			rewind_pressed = 0;
			Rewind_sync_encode_state();
			rewinding = 0;
			ff_paused_by_rewind_hold = 0;
		}
	}
}

// ensure live gameplay immediately picks up scaler/effect changes triggered via shortcuts
static void apply_live_video_reset(void) {
	// defer work to the video thread: mark scaler dirty (shader reset not needed here)
	renderer.dst_p = 0;
	// If shaders are disabled (0 passes), force a reset so the default pipeline rebuilds
	if (config.shaders.options[SH_NROFSHADERS].value == 0) {
		GFX_resetShaders();
		shader_reset_suppressed = 0;
	} else {
		shader_reset_suppressed = 1; // skip reset for >0 shader pipelines
	}
}

char** list_files_in_folder(const char* folderPath, int* fileCount, const char* defaultElement, const char* extensionFilter) {
    DIR* dir = opendir(folderPath);
    if (!dir) {
        perror("opendir");
        return NULL;
    }

    struct dirent* entry;
    struct stat fileStat;
    char** fileList = NULL;
    *fileCount = 0;

	if(defaultElement) {
		fileList = malloc(sizeof(char* ) * 2);
		fileList[0] = strdup(defaultElement);
		fileList[1] = NULL;
		(*fileCount)++;
	}

    while ((entry = readdir(dir)) != NULL) {
		// skip all entries starting with ._ (hidden files on macOS)
		if (entry->d_name[0] == '.' && entry->d_name[1] == '_')
			continue;
		// skip macOS .DS_Store files
		if (strcmp(entry->d_name, ".DS_Store") == 0)
			continue;
		
        char fullPath[1024];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", folderPath, entry->d_name);

        if (stat(fullPath, &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
            if (extensionFilter) {
                const char* ext = strrchr(entry->d_name, '.');
                if (!ext || strcmp(ext, extensionFilter) != 0) {
                    continue;
                }
            }

            char** temp = realloc(fileList, sizeof(char*) * (*fileCount + 1)); 
            if (!temp) {
                perror("realloc");
                for (int i = 0; i < *fileCount; ++i) {
                    free(fileList[i]);
                }
                free(fileList);
                closedir(dir);
                return NULL;
            }
            fileList = temp;
            fileList[*fileCount] = strdup(entry->d_name);
            (*fileCount)++;
        }
    }

    closedir(dir);

    // Alphabetical sort
    for (int i = 0; i < *fileCount - 1; ++i) {
        for (int j = i + 1; j < *fileCount; ++j) {
            if (strcmp(fileList[i], fileList[j]) > 0) {
                char* temp = fileList[i];
                fileList[i] = fileList[j];
                fileList[j] = temp;
            }
        }
    }

	// NUll terminate the list
	char** temp = realloc(fileList, sizeof(char*) * (*fileCount + 1));
	if (!temp) {
		perror("realloc");
		for (int i = 0; i < *fileCount; ++i) {
			free(fileList[i]);
		}
		free(fileList);
		return NULL;
	}
	fileList = temp;
	fileList[*fileCount] = NULL;

    return fileList;
}

// CONFIG_WRITE_ALL, CONFIG_WRITE_GAME defined in minarch_internal.h
static void Config_getPath(char* filename, int override) {
	char device_tag[64] = {0};
	if (config.device_tag) sprintf(device_tag,"-%s",config.device_tag);
	if (override) sprintf(filename, "%s/%s%s.cfg", core.config_dir, game.alt_name, device_tag);
	else sprintf(filename, "%s/minarch%s.cfg", core.config_dir, device_tag);
	LOG_info("Config_getPath %s\n", filename);
}
void Config_init(void) {
	if (!config.default_cfg || config.initialized) return;
	
	LOG_info("Config_init\n");
	char* tmp = config.default_cfg;
	char* tmp2;
	char* key;
	
	char button_name[128];
	char button_id[128];
	int i = 0;
	while ((tmp = strstr(tmp, "bind "))) {
		tmp += 5; // tmp now points to the button name (plus the rest of the line)
		key = tmp;
		tmp = strstr(tmp, " = ");
		if (!tmp) break;
		
		int len = tmp-key;
		strncpy(button_name, key, len);
		button_name[len] = '\0';
		
		tmp += 3;
		strncpy(button_id, tmp, 128);
		tmp2 = strchr(button_id, '\n');
		if (!tmp2) tmp2 = strchr(button_id, '\r');
		if (tmp2) *tmp2 = '\0';
		
		int retro_id = -1;
		int local_id = -1;
		
		tmp2 = strrchr(button_id, ':');
		int remap = 0;
		if (tmp2) {
			for (int j=0; button_label_mapping[j].name; j++) {
				ButtonMapping* button = &button_label_mapping[j];
				if (!strcmp(tmp2+1,button->name)) {
					retro_id = button->retro;
					break;
				}
			}
			*tmp2 = '\0';
		}
		for (int j=0; button_label_mapping[j].name; j++) {
			ButtonMapping* button = &button_label_mapping[j];
			if (!strcmp(button_id,button->name)) {
				local_id = button->local;
				if (retro_id==-1) retro_id = button->retro;
				break;
			}
		}
		
		tmp += strlen(button_id); // prepare to continue search
		
		LOG_info("\tbind %s (%s) %i:%i\n", button_name, button_id, local_id, retro_id);
		
		// TODO: test this without a final line return
		tmp2 = calloc(strlen(button_name)+1, sizeof(char));
		strcpy(tmp2, button_name);
		ButtonMapping* button = &core_button_mapping[i++];
		button->name = tmp2;
		button->retro = retro_id;
		button->local = local_id;
	};

	// populate shader presets
	// TODO: None option?
	int preset_filecount;
	char** preset_filelist = list_files_in_folder(SHADERS_FOLDER, &preset_filecount, NULL, ".cfg");
	config.shaders.options[SH_SHADERS_PRESET].values = preset_filelist;
	
	// populate shader options
	// TODO: None option?
	// TODO: Why do we do this twice? (see OptionShaders_openMenu)
	int filecount;
	char** filelist = list_files_in_folder(SHADERS_FOLDER "/glsl", &filecount, NULL, NULL);

	config.shaders.options[SH_SHADER1].values = filelist;
	config.shaders.options[SH_SHADER1].labels = filelist;
	config.shaders.options[SH_SHADER1].count = filecount;

	config.shaders.options[SH_SHADER2].values = filelist;
	config.shaders.options[SH_SHADER2].labels = filelist;
	config.shaders.options[SH_SHADER2].count = filecount;

	config.shaders.options[SH_SHADER3].values = filelist;
	config.shaders.options[SH_SHADER3].labels = filelist;
	config.shaders.options[SH_SHADER3].count = filecount;
	
	char overlaypath[MAX_PATH];
	snprintf(overlaypath, sizeof(overlaypath), "%s/%s", OVERLAYS_FOLDER, core.tag);
	char** overlaylist = list_files_in_folder(overlaypath, &filecount, "None", NULL);

	if (overlaylist) {
		config.frontend.options[FE_OPT_OVERLAY].labels = overlaylist;
		config.frontend.options[FE_OPT_OVERLAY].values = overlaylist;
		config.frontend.options[FE_OPT_OVERLAY].count = filecount;
	}
	config.initialized = 1;
}
void Config_quit(void) {
	if (!config.initialized) return;
	for (int i=0; core_button_mapping[i].name; i++) {
		free(core_button_mapping[i].name);
	}
}
static void Config_readOptionsString(char* cfg) {
	if (!cfg) return;

	LOG_info("Config_readOptions\n");
	char key[256];
	char value[256];
	for (int i=0; config.frontend.options[i].key; i++) {
		Option* option = &config.frontend.options[i];
		if (!Config_getValue(cfg, option->key, value, &option->lock)) continue;
		OptionList_setOptionValue(&config.frontend, option->key, value);
		Config_syncFrontend(option->key, option->value);
	}
	
	if (has_custom_controllers && Config_getValue(cfg,"minarch_gamepad_type",value,NULL)) {
		gamepad_type = strtol(value, NULL, 0);
		int device = strtol(gamepad_values[gamepad_type], NULL, 0);
		core.set_controller_port_device(0, device);
	}
	for (int i=0; config.core.options[i].key; i++) {
		Option* option = &config.core.options[i];
		// LOG_info("%s\n",option->key);
		if (!Config_getValue(cfg, option->key, value, &option->lock)) continue;
		OptionList_setOptionValue(&config.core, option->key, value);
	}
	for (int i=0; config.shaders.options[i].key; i++) {
		Option* option = &config.shaders.options[i];
		if (!Config_getValue(cfg, option->key, value, &option->lock)) continue;
		OptionList_setOptionValue(&config.shaders, option->key, value);
	}
	for (int y=0; y < config.shaders.options[SH_NROFSHADERS].value; y++) {
		if(config.shaderpragmas[y].count > 0) {
			for (int i=0; config.shaderpragmas[y].options[i].key; i++) {
				Option* option = &config.shaderpragmas[y].options[i];
				if (!Config_getValue(cfg, option->key, value, &option->lock)) continue;
				OptionList_setOptionValue(&config.shaderpragmas[y], option->key, value);
			}
		}
	}
}
static void Config_readControlsString(char* cfg) {
	if (!cfg) return;

	LOG_info("Config_readControlsString\n");
	
	char key[256];
	char value[256];
	char* tmp;
	for (int i=0; config.controls[i].name; i++) {
		ButtonMapping* mapping = &config.controls[i];
		sprintf(key, "bind %s", mapping->name);
		sprintf(value, "NONE");
		
		if (!Config_getValue(cfg, key, value, NULL)) continue;
		if ((tmp = strrchr(value, ':'))) *tmp = '\0'; // this is a binding artifact in default.cfg, ignore
		
		int id = -1;
		for (int j=0; button_labels[j]; j++) {
			if (!strcmp(button_labels[j],value)) {
				id = j - 1;
				break;
			}
		}
		// LOG_info("\t%s (%i)\n", value, id);
		
		int mod = 0;
		if (id>=LOCAL_BUTTON_COUNT) {
			id -= LOCAL_BUTTON_COUNT;
			mod = 1;
		}
		
		mapping->local = id;
		mapping->mod = mod;
	}
	
	for (int i=0; config.shortcuts[i].name; i++) {
		ButtonMapping* mapping = &config.shortcuts[i];
		sprintf(key, "bind %s", mapping->name);
		sprintf(value, "NONE");

		if (!Config_getValue(cfg, key, value, NULL)) continue;
		
		int id = -1;
		for (int j=0; button_labels[j]; j++) {
			if (!strcmp(button_labels[j],value)) {
				id = j - 1;
				break;
			}
		}
		
		int mod = 0;
		if (id>=LOCAL_BUTTON_COUNT) {
			id -= LOCAL_BUTTON_COUNT;
			mod = 1;
		}
		// LOG_info("shortcut %s:%s (%i:%i)\n", key,value, id, mod);

		mapping->local = id;
		mapping->mod = mod;
	}
}
static void configureAmbientOption(void); // defined next to ambient_labels below
static void Minarch_applyFrontendI18n(void); // NextUI-CN, defined after the label tables

void Config_load(void) {
	LOG_info("Config_load\n");
	
	config.device_tag = getenv("DEVICE");
	LOG_info("config.device_tag %s\n", config.device_tag);
	
	// update for crop overscan support
	Option* scaling_option = &config.frontend.options[FE_OPT_SCALING];
	scaling_option->desc = getScreenScalingDesc();
	scaling_option->count = getScreenScalingCount();
	if (!GFX_supportsOverscan()) {
		scaling_labels[4] = NULL;
	}

	configureAmbientOption();
	Minarch_applyFrontendI18n(); // NextUI-CN

	char* system_path = SYSTEM_PATH "/system.cfg";
	
	char device_system_path[MAX_PATH] = {0};
	if (config.device_tag) sprintf(device_system_path, SYSTEM_PATH "/system-%s.cfg", config.device_tag);
	
	if (config.device_tag && exists(device_system_path)) {
		LOG_info("usng device_system_path: %s\n", device_system_path);
		config.system_cfg = allocFile(device_system_path);
	}
	else if (exists(system_path)) config.system_cfg = allocFile(system_path);
	else config.system_cfg = NULL;
	
	// LOG_info("config.system_cfg: %s\n", config.system_cfg);
	
	char default_path[MAX_PATH];
	getEmuPath((char *)core.tag, default_path);
	char* tmp = strrchr(default_path, '/');
	strcpy(tmp,"/default.cfg");

	char device_default_path[MAX_PATH] = {0};
	if (config.device_tag) {
		getEmuPath((char *)core.tag, device_default_path);
		tmp = strrchr(device_default_path, '/');
		char filename[64];
		sprintf(filename,"/default-%s.cfg", config.device_tag);
		strcpy(tmp,filename);
	}
	
	if (config.device_tag && exists(device_default_path)) {
		LOG_info("usng device_default_path: %s\n", device_default_path);
		config.default_cfg = allocFile(device_default_path);
	}
	else if (exists(default_path)) config.default_cfg = allocFile(default_path);
	else config.default_cfg = NULL;
	
	// LOG_info("config.default_cfg: %s\n", config.default_cfg);
	
	char path[MAX_PATH];
	config.loaded = CONFIG_NONE;
	int override = 0;
	Config_getPath(path, CONFIG_WRITE_GAME);
	if (exists(path)) override = 1; 
	if (!override) Config_getPath(path, CONFIG_WRITE_ALL);
	
	config.user_cfg = allocFile(path);
	if (!config.user_cfg) return;
	
	LOG_info("using user config: %s\n", path);
	
	config.loaded = override ? CONFIG_GAME : CONFIG_CONSOLE;
}
void Config_free(void) {
	if (config.system_cfg) free(config.system_cfg);
	if (config.default_cfg) free(config.default_cfg);
	if (config.user_cfg) free(config.user_cfg);
}
void Config_readOptions(void) {
	Config_readOptionsString(config.system_cfg);
	Config_readOptionsString(config.default_cfg);
	Config_readOptionsString(config.user_cfg);
}
void Config_readControls(void) {
	Config_readControlsString(config.default_cfg);
	Config_readControlsString(config.user_cfg);
}
void Config_write(int override) {
	char path[MAX_PATH];
	// sprintf(path, "%s/%s.cfg", core.config_dir, game.alt_name);
	Config_getPath(path, CONFIG_WRITE_GAME);
	
	if (!override) {
		if (config.loaded==CONFIG_GAME) unlink(path);
		Config_getPath(path, CONFIG_WRITE_ALL);
	}
	config.loaded = override ? CONFIG_GAME : CONFIG_CONSOLE;
	
	FILE *file = fopen(path, "wb");
	if (!file) return;
	
	for (int i=0; config.frontend.options[i].key; i++) {
		Option* option = &config.frontend.options[i];
		int count = 0;
		while ( option->values &&  option->values[count]) count++;
		if (option->value >= 0 && option->value < count) {
			fprintf(file, "%s = %s\n", option->key, option->values[option->value]);
		}
	}
	for (int i=0; config.core.options[i].key; i++) {
		Option* option = &config.core.options[i];
		fprintf(file, "%s = %s\n", option->key, option->values[option->value]);
	}
	for (int i=0; config.shaders.options[i].key; i++) {
		Option* option = &config.shaders.options[i];
		int count = 0;
		while ( option->values &&  option->values[count]) count++;
		if (option->value >= 0 && option->value < count) {
			fprintf(file, "%s = %s\n", option->key, option->values[option->value]);
		}
	}
	for (int y=0; y < config.shaders.options[SH_NROFSHADERS].value; y++) {
		for (int i=0; config.shaderpragmas[y].options[i].key; i++) {
			Option* option = &config.shaderpragmas[y].options[i];
			int count = 0;
			while ( option->values &&  option->values[count]) count++;
			if (option->value >= 0 && option->value < count) {
				fprintf(file, "%s = %s\n", option->key, option->values[option->value]);
			}
		}
	}
	
	if (has_custom_controllers) fprintf(file, "%s = %i\n", "minarch_gamepad_type", gamepad_type);
	
	for (int i=0; config.controls[i].name; i++) {
		ButtonMapping* mapping = &config.controls[i];
		int j = mapping->local + 1;
		if (mapping->mod) j += LOCAL_BUTTON_COUNT;
		fprintf(file, "bind %s = %s\n", mapping->name, button_labels[j]);
	}
	for (int i=0; config.shortcuts[i].name; i++) {
		ButtonMapping* mapping = &config.shortcuts[i];
		int j = mapping->local + 1;
		if (mapping->mod) j += LOCAL_BUTTON_COUNT;
		fprintf(file, "bind %s = %s\n", mapping->name, button_labels[j]);
	}
	
	fclose(file);
	sync();
}
void Config_restore(void) {
	char path[MAX_PATH];
	if (config.loaded==CONFIG_GAME) {
		if (config.device_tag) sprintf(path, "%s/%s-%s.cfg", core.config_dir, game.alt_name, config.device_tag);
		else sprintf(path, "%s/%s.cfg", core.config_dir, game.alt_name);
		unlink(path);
		LOG_info("deleted game config: %s\n", path);
	}
	else if (config.loaded==CONFIG_CONSOLE) {
		if (config.device_tag) sprintf(path, "%s/minarch-%s.cfg", core.config_dir, config.device_tag);
		else sprintf(path, "%s/minarch.cfg", core.config_dir);
		unlink(path);
		LOG_info("deleted console config: %s\n", path);
	}
	config.loaded = CONFIG_NONE;
	
	for (int i=0; config.frontend.options[i].key; i++) {
		Option* option = &config.frontend.options[i];
		option->value = option->default_value;
		Config_syncFrontend(option->key, option->value);
	}
	for (int i=0; config.core.options[i].key; i++) {
		Option* option = &config.core.options[i];
		option->value = option->default_value;
	}
	for (int i=0; config.shaders.options[i].key; i++) {
		Option* option = &config.shaders.options[i];
		option->value = option->default_value;
	}
	config.core.changed = 1; // let the core know
	
	if (has_custom_controllers) {
		gamepad_type = 0;
		core.set_controller_port_device(0, RETRO_DEVICE_JOYPAD);
	}

	for (int i=0; config.controls[i].name; i++) {
		ButtonMapping* mapping = &config.controls[i];
		mapping->local = mapping->default_;
		mapping->mod = 0;
	}
	for (int i=0; config.shortcuts[i].name; i++) {
		ButtonMapping* mapping = &config.shortcuts[i];
		mapping->local = BTN_ID_NONE;
		mapping->mod = 0;
	}
	
	Config_load();
	Config_readOptions();
	Config_readControls();
	Config_free();
	
	renderer.dst_p = 0;
}

void readShadersPreset(int i) {
	char shaderspath[MAX_PATH] = {0};
	sprintf(shaderspath, SHADERS_FOLDER "/%s", config.shaders.options[SH_SHADERS_PRESET].values[i]);
	LOG_info("read shaders preset %s\n",shaderspath);
	if (exists(shaderspath)) {
		config.shaders_preset = allocFile(shaderspath);
		Config_readOptionsString(config.shaders_preset);
	}
	else config.shaders_preset = NULL;
}

void loadShaderSettings(int i) {
	int menucount = 0;
	config.shaderpragmas[i].options = calloc(32 + 1, sizeof(Option));
	ShaderParam *params = PLAT_getShaderPragmas(i);
	if(params == NULL) return;
	for (int j = 0; j < 32; j++) {
	
		if (params[j].step == 0.0f) {
			// Prevent division by zero; skip this parameter or set steps to 1
			continue;
		}

		if (!params[j].name || strlen(params[j].name) == 0) {
			// Skip invalid parameter names
			continue;
		}
		config.shaderpragmas[i].options[menucount].key = params[j].name;
		config.shaderpragmas[i].options[menucount].name = params[j].name;
		config.shaderpragmas[i].options[menucount].desc = params[j].name;
		config.shaderpragmas[i].options[menucount].default_value = params[j].def;
		
		int steps = (int)((params[j].max - params[j].min) / params[j].step) + 1;
		config.shaderpragmas[i].options[menucount].values = malloc(sizeof(char *) * (steps + 1));
		config.shaderpragmas[i].options[menucount].labels = malloc(sizeof(char *) * (steps + 1));
		for (int s = 0; s < steps; s++) {
			float val = params[j].min + s * params[j].step;
			char *str = malloc(16);
			snprintf(str, 16, "%.2f", val);
			config.shaderpragmas[i].options[menucount].values[s] = str;
			config.shaderpragmas[i].options[menucount].labels[s] = str;
			if (fabs(params[j].value - val) < 0.001f)
				config.shaderpragmas[i].options[menucount].value = s;
		}
		config.shaderpragmas[i].options[menucount].count = steps;
		config.shaderpragmas[i].options[menucount].values[steps] = NULL;
		config.shaderpragmas[i].options[menucount].labels[steps] = NULL;
		menucount++;
		
	}
	config.shaderpragmas[i].count = menucount;
}

void Config_syncShaders(char* key, int value) {
	int i = -1;
	if (exactMatch(key,config.shaders.options[SH_SHADERS_PRESET].key)) {
		readShadersPreset(value);
		i = SH_SHADERS_PRESET;
	}
	else if (exactMatch(key,config.shaders.options[SH_NROFSHADERS].key)) {
		GFX_setShaders(value);
		i = SH_NROFSHADERS;
	}
	else if (exactMatch(key, config.shaders.options[SH_SHADER1].key)) {
		char** shaderList = config.shaders.options[SH_SHADER1].values;
		if (shaderList) {
			LOG_info("minarch: updating shader 1 - %i\n",value);
			int count = 0;
			while (shaderList && shaderList[count]) count++;
			if (value >= 0 && value < count) {
				GFX_updateShader(0, shaderList[value], NULL, NULL,NULL,NULL);
				i = SH_SHADER1;
			} 
		}
		loadShaderSettings(0);
	}
	else if (exactMatch(key,config.shaders.options[SH_SHADER1_FILTER].key)) {
		GFX_updateShader(0,NULL,NULL,&value,NULL,NULL);
		i = SH_SHADER1_FILTER;
	}
	else if (exactMatch(key,config.shaders.options[SH_SRCTYPE1].key)) {
		GFX_updateShader(0,NULL,NULL,NULL,NULL,&value);
		i = SH_SRCTYPE1;
	}
	if (exactMatch(key,config.shaders.options[SH_SCALETYPE1].key)) {
		GFX_updateShader(0,NULL,NULL,NULL,&value,NULL);
		i = SH_SCALETYPE1;
	}
	else if (exactMatch(key,config.shaders.options[SH_UPSCALE1].key)) {
		GFX_updateShader(0,NULL,&value,NULL,NULL,NULL);
		i = SH_UPSCALE1;
	}
	else if (exactMatch(key, config.shaders.options[SH_SHADER2].key)) {
		char** shaderList = config.shaders.options[SH_SHADER2].values;
		if (shaderList) {
			LOG_info("minarch: updating shader 2 - %i\n",value);
			int count = 0;
			while (shaderList && shaderList[count]) count++;
			if (value >= 0 && value < count) {
				GFX_updateShader(1, shaderList[value], NULL, NULL,NULL,NULL);
				i = SH_SHADER2;
			}
		}
		loadShaderSettings(1);
	}
	else if (exactMatch(key,config.shaders.options[SH_SHADER2_FILTER].key)) {
		GFX_updateShader(1,NULL,NULL,&value,NULL,NULL);
		i = SH_SHADER2_FILTER;
	}
	else if (exactMatch(key,config.shaders.options[SH_SRCTYPE2].key)) {
		GFX_updateShader(1,NULL,NULL,NULL,NULL,&value);
		i = SH_SRCTYPE2;
	}
	else if (exactMatch(key,config.shaders.options[SH_SCALETYPE2].key)) {
		GFX_updateShader(1,NULL,NULL,NULL,&value,NULL);
		i = SH_SCALETYPE2;
	}
	else if (exactMatch(key,config.shaders.options[SH_UPSCALE2].key)) {
		GFX_updateShader(1,NULL,&value,NULL,NULL,NULL);
		i = SH_UPSCALE2;
	}
	else if (exactMatch(key, config.shaders.options[SH_SHADER3].key)) {
		char** shaderList = config.shaders.options[SH_SHADER3].values;
		if (shaderList) {
			LOG_info("minarch: updating shader 3 - %i\n",value);
			int count = 0;
			while (shaderList && shaderList[count]) count++;
			if (value >= 0 && value < count) {
				GFX_updateShader(2, shaderList[value], NULL, NULL,NULL,NULL);
				i = SH_SHADER3;
			}
		}
		loadShaderSettings(2);
	}
	else if (exactMatch(key,config.shaders.options[SH_SHADER3_FILTER].key)) {
		GFX_updateShader(2,NULL,NULL,&value,NULL,NULL);
		i = SH_SHADER3_FILTER;
	}
	if (exactMatch(key,config.shaders.options[SH_SRCTYPE3].key)) {
		GFX_updateShader(2,NULL,NULL,NULL,NULL,&value);
		i = SH_SRCTYPE3;
	}
	else if (exactMatch(key,config.shaders.options[SH_SCALETYPE3].key)) {
		GFX_updateShader(2,NULL,NULL,NULL,&value,NULL);
		i = SH_SCALETYPE3;
	}
	else if (exactMatch(key,config.shaders.options[SH_UPSCALE3].key)) {
		GFX_updateShader(2,NULL,&value,NULL,NULL,NULL);
		i = SH_UPSCALE3;
	}
	
	if (i==-1) return;
	Option* option = &config.shaders.options[i];
	option->value = value;
}

////////

void applyShaderSettings() {
	for (int y=0; y < config.shaders.options[SH_NROFSHADERS].value; y++) {
		ShaderParam *params = PLAT_getShaderPragmas(y);
		if (params == NULL) {
			break;
		}
		for (int i=0; i < config.shaderpragmas[y].count; i++) {
			for (int j = 0; j < 32; j++) {
				if (exactMatch(params[j].name, config.shaderpragmas[y].options[i].key)) {
					params[j].value = strtof(config.shaderpragmas[y].options[i].values[config.shaderpragmas[y].options[i].value], NULL);
				}
			}
		}
	}
}
void initShaders() {
	for (int i=0; config.shaders.options[i].key; i++) {
		if(i!=SH_SHADERS_PRESET) {
			Option* option = &config.shaders.options[i];;
			Config_syncShaders(option->key, option->value);
		}
	}
}

/* -----------------------------------------------------------------------
   Config data: label arrays, button mappings, and struct Config initializer.
   Moved from minarch.c; these are the static data backing the config module.
   ----------------------------------------------------------------------- */

char* onoff_labels[] = {
	"Off",
	"On",
	NULL
};
char* scaling_labels[] = {
	"Native",
	"Aspect",
	"Aspect Screen",
	"Fullscreen",
	"Cropped",
	NULL
};
static char* resample_labels[] = {
	"Low",
	"Medium",
	"High",
	"Max",
	NULL
};
static char* rewind_enable_labels[] = {
	"Off",
	"On",
	NULL
};
static char* rewind_buffer_labels[] = {
	"8",
	"16",
	"32",
	"64",
	"128",
	"256",
	NULL
};
static char* rewind_granularity_values[] = {
	"16",
	"22",
	"25",
	"33",
	"50",
	"66",
	"100",
	"150",
	"200",
	"300",
	"450",
	"600",
	NULL
};
static char* rewind_granularity_labels[] = {
	"16 ms (~60 fps)",
	"22 ms (~45 fps)",
	"25 ms (~40 fps)",
	"33 ms (~30 fps)",
	"50 ms (~20 fps)",
	"66 ms (~15 fps)",
	"100 ms (~10 fps)",
	"150 ms (~7 fps)",
	"200 ms (~5 fps)",
	"300 ms",
	"450 ms",
	"600 ms",
	NULL
};
static char* rewind_compression_accel_values[] = {
	"1",
	"2",
	"4",
	"8",
	"12",
	NULL
};
static char* rewind_compression_accel_labels[] = {
	"1 (best ratio)",
	"2 (default)",
	"4 (fast)",
	"8 (faster)",
	"12 (fastest)",
	NULL
};
static char* ambient_labels[] = {
	"Off",
	"All",
	"Top",
	"FN",
	"LR",
	"Top/LR",
	NULL
};

static char* effect_labels[] = {
	"None",
	"Line",
	"Grid",
	NULL
};
static char* overlay_labels[] = {
	"None",
	NULL
};
// static char* sharpness_labels[] = {
// 	"Sharp",
// 	"Crisp",
// 	"Soft",
// 	NULL
// };
static char* sharpness_labels[] = {
	"NEAREST",
	"LINEAR",
	NULL
};
char* sync_ref_labels[] = {
	"Auto",
	"Screen",
	"Native",
	NULL
};
static char* max_ff_labels[] = {
	"None",
	"2x",
	"3x",
	"4x",
	"5x",
	"6x",
	"7x",
	"8x",
	NULL,
};
static char* offset_labels[] = {
	"-64",
	"-63",
	"-62",
	"-61",
	"-60",
	"-59",
	"-58",
	"-57",
	"-56",
	"-55",
	"-54",
	"-53",
	"-52",
	"-51",
	"-50",
	"-49",
	"-48",
	"-47",
	"-46",
	"-45",
	"-44",
	"-43",
	"-42",
	"-41",
	"-40",
	"-39",
	"-38",
	"-37",
	"-36",
	"-35",
	"-34",
	"-33",
	"-32",
	"-31",
	"-30",
	"-29",
	"-28",
	"-27",
	"-26",
	"-25",
	"-24",
	"-23",
	"-22",
	"-21",
	"-20",
	"-19",
	"-18",
	"-17",
	"-16",
	"-15",
	"-14",
	"-13",
	"-12",
	"-11",
	"-10",
	"-9",
	"-8",
	"-7",
	"-6",
	"-5",
	"-4",
	"-3",
	"-2",
	"-1",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"10",
	"11",
	"12",
	"13",
	"14",
	"15",
	"16",
	"17",
	"18",
	"19",
	"20",
	"21",
	"22",
	"23",
	"24",
	"25",
	"26",
	"27",
	"28",
	"29",
	"30",
	"31",
	"32",
	"33",
	"34",
	"35",
	"36",
	"37",
	"38",
	"39",
	"40",
	"41",
	"42",
	"43",
	"44",
	"45",
	"46",
	"47",
	"48",
	"49",
	"50",
	"51",
	"52",
	"53",
	"54",
	"55",
	"56",
	"57",
	"58",
	"59",
	"60",
	"61",
	"62",
	"63",
	"64",
	NULL,
};
static char* nrofshaders_labels[] = {
	"off",
	"1",
	"2",
	"3",
	NULL
};
static char* shupscale_labels[] = {
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"screen",
	NULL
};
static char* shfilter_labels[] = {
	"NEAREST",
	"LINEAR",
	NULL
};
static char* shscaletype_labels[] = {
	"source",
	"relative",
	NULL
};

// SYNC_SRC_AUTO, SYNC_SRC_SCREEN, SYNC_SRC_CORE defined in minarch_internal.h
// SH_EXTRASETTINGS..SH_NONE defined in minarch_internal.h


ButtonMapping default_button_mapping[] = { // used if pak.cfg doesn't exist or doesn't have bindings
	{"Up",			RETRO_DEVICE_ID_JOYPAD_UP,		BTN_ID_DPAD_UP},
	{"Down",		RETRO_DEVICE_ID_JOYPAD_DOWN,	BTN_ID_DPAD_DOWN},
	{"Left",		RETRO_DEVICE_ID_JOYPAD_LEFT,	BTN_ID_DPAD_LEFT},
	{"Right",		RETRO_DEVICE_ID_JOYPAD_RIGHT,	BTN_ID_DPAD_RIGHT},
	{"A Button",	RETRO_DEVICE_ID_JOYPAD_A,		BTN_ID_A},
	{"B Button",	RETRO_DEVICE_ID_JOYPAD_B,		BTN_ID_B},
	{"X Button",	RETRO_DEVICE_ID_JOYPAD_X,		BTN_ID_X},
	{"Y Button",	RETRO_DEVICE_ID_JOYPAD_Y,		BTN_ID_Y},
	{"Start",		RETRO_DEVICE_ID_JOYPAD_START,	BTN_ID_START},
	{"Select",		RETRO_DEVICE_ID_JOYPAD_SELECT,	BTN_ID_SELECT},
	{"L1 Button",	RETRO_DEVICE_ID_JOYPAD_L,		BTN_ID_L1},
	{"R1 Button",	RETRO_DEVICE_ID_JOYPAD_R,		BTN_ID_R1},
	{"L2 Button",	RETRO_DEVICE_ID_JOYPAD_L2,		BTN_ID_L2},
	{"R2 Button",	RETRO_DEVICE_ID_JOYPAD_R2,		BTN_ID_R2},
	{"L3 Button",	RETRO_DEVICE_ID_JOYPAD_L3,		BTN_ID_L3},
	{"R3 Button",	RETRO_DEVICE_ID_JOYPAD_R3,		BTN_ID_R3},
	{NULL,0,0}
};

ButtonMapping core_button_mapping[RETRO_BUTTON_COUNT+1] = {0};

static const char* device_button_names[LOCAL_BUTTON_COUNT] = {
	[BTN_ID_DPAD_UP]	= "UP",
	[BTN_ID_DPAD_DOWN]	= "DOWN",
	[BTN_ID_DPAD_LEFT]	= "LEFT",
	[BTN_ID_DPAD_RIGHT]	= "RIGHT",
	[BTN_ID_SELECT]		= "SELECT",
	[BTN_ID_START]		= "START",
	[BTN_ID_Y]			= "Y",
	[BTN_ID_X]			= "X",
	[BTN_ID_B]			= "B",
	[BTN_ID_A]			= "A",
	[BTN_ID_L1]			= "L1",
	[BTN_ID_R1]			= "R1",
	[BTN_ID_L2]			= "L2",
	[BTN_ID_R2]			= "R2",
	[BTN_ID_L3]			= "L3",
	[BTN_ID_R3]			= "R3",
	[BTN_ID_L4]			= "L4",
	[BTN_ID_R4]			= "R4",
};


// NOTE: these must be in BTN_ID_ order also off by 1 because of NONE (which is -1 in BTN_ID_ land)
char* button_labels[] = {
	"NONE", // displayed by default
	"UP",
	"DOWN",
	"LEFT",
	"RIGHT",
	"A",
	"B",
	"X",
	"Y",
	"START",
	"SELECT",
	"L1",
	"R1",
	"L2",
	"R2",
	"L3",
	"R3",
	"L4",
	"R4",
	"MENU+UP",
	"MENU+DOWN",
	"MENU+LEFT",
	"MENU+RIGHT",
	"MENU+A",
	"MENU+B",
	"MENU+X",
	"MENU+Y",
	"MENU+START",
	"MENU+SELECT",
	"MENU+L1",
	"MENU+R1",
	"MENU+L2",
	"MENU+R2",
	"MENU+L3",
	"MENU+R3",
	"MENU+L4",
	"MENU+R4",
	NULL,
};
static char* overclock_labels[] = {
	"Auto",
	"Performance",
	"Powersave",
	NULL,
};

// TODO: this should be provided by the core
char* gamepad_labels[] = {
	"Standard",
	"DualShock",
	NULL,
};
char* gamepad_values[] = {
	"1",
	"517",
	NULL,
};

// ---- NextUI-CN i18n (display-only labels; *_labels above stay the stable config VALUES) ----
// i18n labels for frontend options (display-only).
// Keep `*_labels` arrays above as stable config VALUES.
static char** i18n_onoff_labels = NULL;
static char** i18n_scaling_labels = NULL;
static char** i18n_resample_labels = NULL;
static char** i18n_ambient_labels = NULL;
static char** i18n_effect_labels = NULL;
static char** i18n_overlay_labels = NULL;
static char** i18n_sync_ref_labels = NULL;
static char** i18n_overclock_labels = NULL;
static char** i18n_max_ff_labels = NULL;
static char** i18n_sharpness_labels = NULL;
char** i18n_button_labels = NULL;
char** i18n_gamepad_labels = NULL;

static char** i18n_nrofshaders_labels = NULL;
static char** i18n_shupscale_labels = NULL;
static char** i18n_shscaletype_labels = NULL;

static char** Minarch_buildI18nLabels(const char** keys, char** fallback_values) {
	int count = 0;
	while (fallback_values && fallback_values[count]) count++;
	char** labels = calloc(count + 1, sizeof(char*));
	for (int i = 0; i < count; i++) {
		const char* key = keys ? keys[i] : NULL;
		if (key && key[0] != '\0') labels[i] = strdup(TR(key));
		else labels[i] = strdup(fallback_values[i]);
	}
	labels[count] = NULL;
	return labels;
}

const char* Minarch_trOr(const char* key, const char* fallback) {
	const char* tr = TR(key);
	return (tr && strcmp(tr, key) != 0) ? tr : fallback;
}

const char* Minarch_translateCategoryLabel(const char* label) {
	if (!label) return label;
	if (!strcasecmp(label, "Video")) return TR("minarch.corecat.video");
	if (!strcasecmp(label, "Audio")) return TR("minarch.corecat.audio");
	if (!strcasecmp(label, "Input")) return TR("minarch.corecat.input");
	if (!strcasecmp(label, "Latency")) return TR("minarch.corecat.latency");
	if (!strcasecmp(label, "System")) return TR("minarch.corecat.system");
	if (!strcasecmp(label, "Core")) return TR("minarch.corecat.core");
	if (!strcasecmp(label, "Emulation")) return TR("minarch.corecat.emulation");
	if (!strcasecmp(label, "CPU")) return TR("minarch.corecat.cpu");
	if (!strcasecmp(label, "Graphics")) return TR("minarch.corecat.graphics");
	if (!strcasecmp(label, "Hacks")) return TR("minarch.corecat.hacks");
	if (!strcasecmp(label, "Misc")) return TR("minarch.corecat.misc");
	return label;
}

static const char* Minarch_translateCommonValue(const char* value) {
	if (!value) return value;
	if (!strcasecmp(value, "off")) return TR("common.off");
	if (!strcasecmp(value, "on")) return TR("common.on");
	if (!strcasecmp(value, "none")) return TR("common.none");
	if (!strcasecmp(value, "auto")) return TR("common.auto");
	if (!strcasecmp(value, "enabled")) return TR("common.enabled");
	if (!strcasecmp(value, "disabled")) return TR("common.disabled");
	if (!strcasecmp(value, "true")) return TR("common.true");
	if (!strcasecmp(value, "false")) return TR("common.false");
	
	// PCSX-ReArmed region
	if (!strcasecmp(value, "ntsc")) return TR("minarch.region.ntsc");
	if (!strcasecmp(value, "pal")) return TR("minarch.region.pal");
	
	// PCSX-ReArmed bios
	if (!strcasecmp(value, "hle")) return TR("minarch.bios.hle");
	
	// PCSX-ReArmed multitap
	if (!strcasecmp(value, "port 1 only")) return TR("minarch.multitap.port1_only");
	if (!strcasecmp(value, "port 2 only")) return TR("minarch.multitap.port2_only");
	if (!strcasecmp(value, "both")) return TR("minarch.multitap.both");
	
	// PCSX-ReArmed negcon response
	if (!strcasecmp(value, "linear")) return TR("minarch.negcon.linear");
	if (!strcasecmp(value, "quadratic")) return TR("minarch.negcon.quadratic");
	if (!strcasecmp(value, "cubic")) return TR("minarch.negcon.cubic");
	
	// PCSX-ReArmed analog axis bounds
	if (!strcasecmp(value, "circle")) return TR("minarch.analog.circle");
	if (!strcasecmp(value, "square")) return TR("minarch.analog.square");
	
	// PCSX-ReArmed threaded rendering
	if (!strcasecmp(value, "sync")) return TR("minarch.render.sync");
	if (!strcasecmp(value, "async")) return TR("minarch.render.async");
	
	// PCSX-ReArmed spu interpolation
	if (!strcasecmp(value, "simple")) return TR("minarch.interpolation.simple");
	if (!strcasecmp(value, "gaussian")) return TR("minarch.interpolation.gaussian");
	if (!strcasecmp(value, "cubic")) return TR("minarch.interpolation.cubic");
	
	// PCSX-ReArmed cd access method
	if (!strcasecmp(value, "precache")) return TR("minarch.cd.precache");
	
	// PCSX-ReArmed dynamic recompiler
	if (!strcasecmp(value, "interpreter")) return TR("minarch.cpu.interpreter");
	
	// Common video filters (NES, SNES, etc.)
	if (!strcasecmp(value, "disabled")) return TR("common.disabled");
	if (!strcasecmp(value, "enabled")) return TR("common.enabled");
	if (!strcasecmp(value, "monochrome")) return TR("minarch.filter.monochrome");
	if (!strcasecmp(value, "composite")) return TR("minarch.filter.composite");
	if (!strcasecmp(value, "s-video")) return TR("minarch.filter.svideo");
	if (!strcasecmp(value, "svideo")) return TR("minarch.filter.svideo");
	if (!strcasecmp(value, "rgb")) return TR("minarch.filter.rgb");
	
	// Common display modes
	if (!strcasecmp(value, "sharp")) return TR("minarch.display.sharp");
	if (!strcasecmp(value, "smooth")) return TR("minarch.display.smooth");
	if (!strcasecmp(value, "grayscale")) return TR("minarch.display.grayscale");
	
	// System types
	if (!strcasecmp(value, "dendy")) return TR("minarch.system.dendy");
	
	// Color modes
	if (!strcasecmp(value, "color")) return TR("minarch.color.color");
	if (!strcasecmp(value, "internal")) return TR("minarch.color.internal");
	
	// Sound quality
	if (!strcasecmp(value, "low")) return TR("minarch.quality.low");
	if (!strcasecmp(value, "medium")) return TR("minarch.quality.medium");
	if (!strcasecmp(value, "high")) return TR("minarch.quality.high");
	if (!strcasecmp(value, "max")) return TR("minarch.quality.max");
	
	// Audio channels
	if (!strcasecmp(value, "mono")) return TR("minarch.audio.mono");
	if (!strcasecmp(value, "stereo")) return TR("minarch.audio.stereo");
	
	// Snes9x specific
	if (!strcasecmp(value, "merge")) return TR("minarch.blend.merge");
	if (!strcasecmp(value, "blur")) return TR("minarch.blend.blur");
	if (!strcasecmp(value, "rf")) return TR("minarch.filter.rf");
	if (!strcasecmp(value, "auto")) return TR("common.auto");
	
	// Common aspect ratios
	if (!strcasecmp(value, "4:3")) return TR("minarch.ratio.4_3");
	if (!strcasecmp(value, "uncorrected")) return TR("minarch.ratio.uncorrected");
	
	// PicoDrive specific
	if (!strcasecmp(value, "3 button pad")) return TR("minarch.pad.3button");
	if (!strcasecmp(value, "6 button pad")) return TR("minarch.pad.6button");
	if (!strcasecmp(value, "par")) return TR("minarch.ratio.par");
	if (!strcasecmp(value, "crt")) return TR("minarch.ratio.crt");
	if (!strcasecmp(value, "low-pass")) return TR("minarch.filter.lowpass");
	if (!strcasecmp(value, "japan ntsc")) return TR("minarch.region.japan_ntsc");
	if (!strcasecmp(value, "japan pal")) return TR("minarch.region.japan_pal");
	if (!strcasecmp(value, "us")) return TR("minarch.region.us");
	if (!strcasecmp(value, "europe")) return TR("minarch.region.europe");
	
	// Overclock percentages
	if (!strcasecmp(value, "+25%")) return TR("minarch.overclock.+25");
	if (!strcasecmp(value, "+50%")) return TR("minarch.overclock.+50");
	if (!strcasecmp(value, "+75%")) return TR("minarch.overclock.+75");
	if (!strcasecmp(value, "+100%")) return TR("minarch.overclock.+100");
	if (!strcasecmp(value, "+200%")) return TR("minarch.overclock.+200");
	if (!strcasecmp(value, "+400%")) return TR("minarch.overclock.+400");
	
	// FCEUMM specific
	if (!strcasecmp(value, "famicom")) return TR("minarch.system.famicom");
	if (!strcasecmp(value, "8:7 par")) return TR("minarch.ratio.8_7_par");
	if (!strcasecmp(value, "2x-postrender")) return TR("minarch.overclock.2x_postrender");
	if (!strcasecmp(value, "2x-vblank")) return TR("minarch.overclock.2x_vblank");
	if (!strcasecmp(value, "lightgun")) return TR("minarch.input.lightgun");
	if (!strcasecmp(value, "touchscreen")) return TR("minarch.input.touchscreen");
	if (!strcasecmp(value, "player 1")) return TR("minarch.turbo.player1");
	if (!strcasecmp(value, "player 2")) return TR("minarch.turbo.player2");
	
	// Gambatte specific
	if (!strcasecmp(value, "auto")) return TR("common.auto");
	if (!strcasecmp(value, "gbc")) return TR("minarch.system.gbc");
	if (!strcasecmp(value, "sgb")) return TR("minarch.system.sgb");
	if (!strcasecmp(value, "internal")) return TR("minarch.color.internal");
	if (!strcasecmp(value, "custom")) return TR("minarch.color.custom");
	if (!strcasecmp(value, "gb - dmg")) return TR("minarch.palette.gb_dmg");
	if (!strcasecmp(value, "gb - pocket")) return TR("minarch.palette.gb_pocket");
	if (!strcasecmp(value, "gb - light")) return TR("minarch.palette.gb_light");
	if (!strcasecmp(value, "gbc only")) return TR("minarch.color.gbc_only");
	if (!strcasecmp(value, "always")) return TR("common.always");
	if (!strcasecmp(value, "accurate")) return TR("minarch.mode.accurate");
	if (!strcasecmp(value, "fast")) return TR("minarch.mode.fast");
	if (!strcasecmp(value, "central")) return TR("minarch.position.central");
	if (!strcasecmp(value, "above screen")) return TR("minarch.position.above_screen");
	if (!strcasecmp(value, "below screen")) return TR("minarch.position.below_screen");
	if (!strcasecmp(value, "gb")) return TR("minarch.system.gb");
	if (!strcasecmp(value, "gba")) return TR("minarch.system.gba");
	
	// mGBA specific
	if (!strcasecmp(value, "autodetect")) return TR("common.autodetect");
	if (!strcasecmp(value, "game boy")) return TR("minarch.system.game_boy");
	if (!strcasecmp(value, "super game boy")) return TR("minarch.system.super_game_boy");
	if (!strcasecmp(value, "game boy color")) return TR("minarch.system.game_boy_color");
	if (!strcasecmp(value, "game boy advance")) return TR("minarch.system.game_boy_advance");
	if (!strcasecmp(value, "remove known")) return TR("minarch.idle.remove_known");
	if (!strcasecmp(value, "detect and remove")) return TR("minarch.idle.detect_and_remove");
	if (!strcasecmp(value, "don't remove")) return TR("minarch.idle.dont_remove");
	
	// PCE FAST specific
	if (!strcasecmp(value, "system card 3")) return TR("minarch.bios.system_card_3");
	if (!strcasecmp(value, "games express")) return TR("minarch.bios.games_express");
	if (!strcasecmp(value, "system card 1")) return TR("minarch.bios.system_card_1");
	if (!strcasecmp(value, "system card 2")) return TR("minarch.bios.system_card_2");
	if (!strcasecmp(value, "fast")) return TR("minarch.speed.fast");
	if (!strcasecmp(value, "medium")) return TR("minarch.speed.medium");
	if (!strcasecmp(value, "slow")) return TR("minarch.speed.slow");
	
	// Virtual Boy specific
	if (!strcasecmp(value, "red & blue")) return TR("minarch.anaglyph.red_blue");
	if (!strcasecmp(value, "red & cyan")) return TR("minarch.anaglyph.red_cyan");
	if (!strcasecmp(value, "red & electric cyan")) return TR("minarch.anaglyph.red_electric_cyan");
	if (!strcasecmp(value, "red & green")) return TR("minarch.anaglyph.red_green");
	if (!strcasecmp(value, "green & magenta")) return TR("minarch.anaglyph.green_magenta");
	if (!strcasecmp(value, "yellow & blue")) return TR("minarch.anaglyph.yellow_blue");
	if (!strcasecmp(value, "black & red")) return TR("minarch.palette.black_red");
	if (!strcasecmp(value, "black & white")) return TR("minarch.palette.black_white");
	if (!strcasecmp(value, "invert x")) return TR("minarch.invert.x");
	if (!strcasecmp(value, "invert y")) return TR("minarch.invert.y");
	if (!strcasecmp(value, "invert both")) return TR("minarch.invert.both");
	
	// Stella2014 (Atari 2600) specific
	if (!strcasecmp(value, "simple")) return TR("minarch.blend.simple");
	if (!strcasecmp(value, "ghosting (65%)")) return TR("minarch.ghosting.65");
	if (!strcasecmp(value, "ghosting (75%)")) return TR("minarch.ghosting.75");
	if (!strcasecmp(value, "ghosting (85%)")) return TR("minarch.ghosting.85");
	if (!strcasecmp(value, "ghosting (95%)")) return TR("minarch.ghosting.95");
	
	// Gearcoleco (ColecoVision) specific
	if (!strcasecmp(value, "1:1 par")) return TR("minarch.ratio.1_1_par");
	if (!strcasecmp(value, "4:3 dar")) return TR("minarch.ratio.4_3_dar");
	if (!strcasecmp(value, "16:9 dar")) return TR("minarch.ratio.16_9_dar");
	if (!strcasecmp(value, "16:10 dar")) return TR("minarch.ratio.16_10_dar");
	if (!strcasecmp(value, "top+bottom")) return TR("minarch.overscan.top_bottom");
	if (!strcasecmp(value, "full (284 width)")) return TR("minarch.overscan.full_284");
	if (!strcasecmp(value, "full (320 width)")) return TR("minarch.overscan.full_320");
	if (!strcasecmp(value, "super action controller")) return TR("minarch.spinner.super_action");
	if (!strcasecmp(value, "wheel controller")) return TR("minarch.spinner.wheel");
	if (!strcasecmp(value, "roller controller")) return TR("minarch.spinner.roller");
	
	// BlueMSX (MSX/SVI/ColecoVision/SG-1000) specific
	if (!strcasecmp(value, "msx")) return TR("minarch.system.msx");
	if (!strcasecmp(value, "msxturbor")) return TR("minarch.system.msx_turbo_r");
	if (!strcasecmp(value, "msx2")) return TR("minarch.system.msx2");
	if (!strcasecmp(value, "msx2+")) return TR("minarch.system.msx2_plus");
	if (!strcasecmp(value, "sega - sg-1000")) return TR("minarch.system.sega_sg1000");
	if (!strcasecmp(value, "sega - sc-3000")) return TR("minarch.system.sega_sc3000");
	if (!strcasecmp(value, "sega - sf-7000")) return TR("minarch.system.sega_sf7000");
	if (!strcasecmp(value, "svi - spectravideo svi-318")) return TR("minarch.system.svi_318");
	if (!strcasecmp(value, "svi - spectravideo svi-328")) return TR("minarch.system.svi_328");
	if (!strcasecmp(value, "svi - spectravideo svi-328 mk2")) return TR("minarch.system.svi_328_mk2");
	if (!strcasecmp(value, "colecovision")) return TR("minarch.system.colecovision");
	if (!strcasecmp(value, "colecovision (spectravideo svi-603)")) return TR("minarch.system.colecovision_svi603");
	if (!strcasecmp(value, "50hz")) return TR("minarch.freq.50hz");
	if (!strcasecmp(value, "60hz")) return TR("minarch.freq.60hz");
	
	// PokeMini (Pokémon Mini) specific
	if (!strcasecmp(value, "dotmatrix")) return TR("minarch.filter.dotmatrix");
	if (!strcasecmp(value, "scanline")) return TR("minarch.filter.scanline");
	if (!strcasecmp(value, "analog")) return TR("minarch.mode.analog");
	if (!strcasecmp(value, "3shades")) return TR("minarch.shades.3");
	if (!strcasecmp(value, "2shades")) return TR("minarch.shades.2");
	if (!strcasecmp(value, "default")) return TR("common.default");
	if (!strcasecmp(value, "old")) return TR("minarch.palette.old");
	if (!strcasecmp(value, "monochrome")) return TR("minarch.palette.monochrome");
	if (!strcasecmp(value, "green")) return TR("minarch.palette.green");
	if (!strcasecmp(value, "green vector")) return TR("minarch.palette.green_vector");
	if (!strcasecmp(value, "red")) return TR("minarch.palette.red");
	if (!strcasecmp(value, "red vector")) return TR("minarch.palette.red_vector");
	if (!strcasecmp(value, "blue lcd")) return TR("minarch.palette.blue_lcd");
	if (!strcasecmp(value, "ledbacklight")) return TR("minarch.palette.led_backlight");
	if (!strcasecmp(value, "girl power")) return TR("minarch.palette.girl_power");
	if (!strcasecmp(value, "blue")) return TR("minarch.palette.blue");
	if (!strcasecmp(value, "blue vector")) return TR("minarch.palette.blue_vector");
	if (!strcasecmp(value, "sepia")) return TR("minarch.palette.sepia");
	if (!strcasecmp(value, "monochrome vector")) return TR("minarch.palette.monochrome_vector");
	
	// Caprice32 (Amstrad CPC) specific
	if (!strcasecmp(value, "onloading")) return TR("minarch.status.onloading");
	if (!strcasecmp(value, "french")) return TR("minarch.lang.french");
	if (!strcasecmp(value, "spanish")) return TR("minarch.lang.spanish");
	if (!strcasecmp(value, "joystick_port2")) return TR("minarch.joystick.port2");
	
	// VICE (Commodore) specific
	if (!strcasecmp(value, "c64 pal auto")) return TR("minarch.model.c64_pal_auto");
	if (!strcasecmp(value, "c64 ntsc auto")) return TR("minarch.model.c64_ntsc_auto");
	if (!strcasecmp(value, "c64c pal auto")) return TR("minarch.model.c64c_pal_auto");
	if (!strcasecmp(value, "c64c ntsc auto")) return TR("minarch.model.c64c_ntsc_auto");
	if (!strcasecmp(value, "c64 pal")) return TR("minarch.model.c64_pal");
	if (!strcasecmp(value, "c64 ntsc")) return TR("minarch.model.c64_ntsc");
	if (!strcasecmp(value, "c64c pal")) return TR("minarch.model.c64c_pal");
	if (!strcasecmp(value, "c64c ntsc")) return TR("minarch.model.c64c_ntsc");
	if (!strcasecmp(value, "c64sx pal")) return TR("minarch.model.c64sx_pal");
	if (!strcasecmp(value, "c64sx ntsc")) return TR("minarch.model.c64sx_ntsc");
	if (!strcasecmp(value, "c128 pal auto")) return TR("minarch.model.c128_pal_auto");
	if (!strcasecmp(value, "c128 ntsc auto")) return TR("minarch.model.c128_ntsc_auto");
	if (!strcasecmp(value, "c128 d pal auto")) return TR("minarch.model.c128_d_pal_auto");
	if (!strcasecmp(value, "c128 d ntsc auto")) return TR("minarch.model.c128_d_ntsc_auto");
	if (!strcasecmp(value, "c128 dcr pal auto")) return TR("minarch.model.c128_dcr_pal_auto");
	if (!strcasecmp(value, "c128 dcr ntsc auto")) return TR("minarch.model.c128_dcr_ntsc_auto");
	if (!strcasecmp(value, "c128 pal")) return TR("minarch.model.c128_pal");
	if (!strcasecmp(value, "c128 ntsc")) return TR("minarch.model.c128_ntsc");
	if (!strcasecmp(value, "vic20 pal auto")) return TR("minarch.model.vic20_pal_auto");
	if (!strcasecmp(value, "vic20 ntsc auto")) return TR("minarch.model.vic20_ntsc_auto");
	if (!strcasecmp(value, "vic20 pal")) return TR("minarch.model.vic20_pal");
	if (!strcasecmp(value, "vic20 ntsc")) return TR("minarch.model.vic20_ntsc");
	if (!strcasecmp(value, "vdc")) return TR("minarch.output.vdc");
	if (!strcasecmp(value, "vicii")) return TR("minarch.output.vicii");
	if (!strcasecmp(value, "soft")) return TR("minarch.reset.soft");
	if (!strcasecmp(value, "hard")) return TR("minarch.reset.hard");
	if (!strcasecmp(value, "freeze")) return TR("minarch.reset.freeze");
	if (!strcasecmp(value, "small")) return TR("minarch.crop.small");
	if (!strcasecmp(value, "medium")) return TR("minarch.crop.medium");
	if (!strcasecmp(value, "maximum")) return TR("minarch.crop.maximum");
	if (!strcasecmp(value, "auto_disable")) return TR("minarch.crop.auto_disable");
	if (!strcasecmp(value, "manual")) return TR("minarch.crop.manual");
	if (!strcasecmp(value, "both")) return TR("minarch.crop.both");
	if (!strcasecmp(value, "horizontal")) return TR("minarch.crop.horizontal");
	if (!strcasecmp(value, "vertical")) return TR("minarch.crop.vertical");
	if (!strcasecmp(value, "16:9")) return TR("minarch.ratio.16_9");
	if (!strcasecmp(value, "16:10")) return TR("minarch.ratio.16_10");
	if (!strcasecmp(value, "5:4")) return TR("minarch.ratio.5_4");
	if (!strcasecmp(value, "enabled_noblur")) return TR("minarch.filter.enabled_noblur");
	if (!strcasecmp(value, "enabled_lowblur")) return TR("minarch.filter.enabled_lowblur");
	if (!strcasecmp(value, "enabled_medblur")) return TR("minarch.filter.enabled_medblur");
	if (!strcasecmp(value, "resid")) return TR("minarch.sid.resid");
	if (!strcasecmp(value, "resid-fp")) return TR("minarch.sid.resid_fp");
	if (!strcasecmp(value, "fastsid")) return TR("minarch.sid.fastsid");
	if (!strcasecmp(value, "jump")) return TR("minarch.face.jump");
	if (!strcasecmp(value, "rotate")) return TR("minarch.face.rotate");
	if (!strcasecmp(value, "rotate_jump")) return TR("minarch.face.rotate_jump");
	if (!strcasecmp(value, "autostart")) return TR("minarch.reset.autostart");
	if (!strcasecmp(value, "warp")) return TR("minarch.warp");
	if (!strcasecmp(value, "mute")) return TR("minarch.mute");
	if (!strcasecmp(value, "disk")) return TR("minarch.media.disk");
	if (!strcasecmp(value, "tape")) return TR("minarch.media.tape");
	if (!strcasecmp(value, "bottom")) return TR("minarch.position.bottom");
	if (!strcasecmp(value, "top")) return TR("minarch.position.top");
	if (!strcasecmp(value, "brown")) return TR("minarch.vkbd.brown");
	if (!strcasecmp(value, "beige")) return TR("minarch.vkbd.beige");
	if (!strcasecmp(value, "dark")) return TR("minarch.vkbd.dark");
	if (!strcasecmp(value, "light")) return TR("minarch.vkbd.light");
	if (!strcasecmp(value, "amber")) return TR("minarch.palette.amber");
	
	// PUAE (Amiga) specific
	if (!strcasecmp(value, "a500og")) return TR("minarch.model.a500og");
	if (!strcasecmp(value, "a500plus")) return TR("minarch.model.a500plus");
	if (!strcasecmp(value, "a1200og")) return TR("minarch.model.a1200og");
	if (!strcasecmp(value, "cd32fr")) return TR("minarch.model.cd32fr");
	if (!strcasecmp(value, "minimum")) return TR("minarch.crop.minimum");
	if (!strcasecmp(value, "smaller")) return TR("minarch.crop.smaller");
	if (!strcasecmp(value, "large")) return TR("minarch.crop.large");
	if (!strcasecmp(value, "larger")) return TR("minarch.crop.larger");
	if (!strcasecmp(value, "lores")) return TR("minarch.resolution.lores");
	if (!strcasecmp(value, "hires")) return TR("minarch.resolution.hires");
	if (!strcasecmp(value, "superhires")) return TR("minarch.resolution.superhires");
	if (!strcasecmp(value, "single")) return TR("minarch.line.single");
	if (!strcasecmp(value, "double")) return TR("minarch.line.double");
	if (!strcasecmp(value, "none")) return TR("minarch.collision.none");
	if (!strcasecmp(value, "sprites")) return TR("minarch.collision.sprites");
	if (!strcasecmp(value, "playfields")) return TR("minarch.collision.playfields");
	if (!strcasecmp(value, "full")) return TR("minarch.collision.full");
	if (!strcasecmp(value, "cd32")) return TR("minarch.vkbd.cd32");
	if (!strcasecmp(value, "anti")) return TR("minarch.interpolation.anti");
	if (!strcasecmp(value, "sinc")) return TR("minarch.interpolation.sinc");
	if (!strcasecmp(value, "emulated")) return TR("minarch.filter.emulated");
	if (!strcasecmp(value, "standard")) return TR("minarch.filter.standard");
	if (!strcasecmp(value, "enhanced")) return TR("minarch.filter.enhanced");
	if (!strcasecmp(value, "internal")) return TR("minarch.sound.internal");
	if (!strcasecmp(value, "files")) return TR("minarch.whdload.files");
	if (!strcasecmp(value, "hdfs")) return TR("minarch.whdload.hdfs");
	if (!strcasecmp(value, "native")) return TR("minarch.whdload.native");
	if (!strcasecmp(value, "config")) return TR("minarch.whdload.config");
	if (!strcasecmp(value, "splash")) return TR("minarch.whdload.splash");
	if (!strcasecmp(value, "rh")) return TR("minarch.interpolation.rh");
	if (!strcasecmp(value, "crux")) return TR("minarch.interpolation.crux");
	
	// FBNeo (Arcade) specific
	if (!strcasecmp(value, "alternate")) return TR("minarch.vertical.alternate");
	if (!strcasecmp(value, "hide with lightgun device")) return TR("minarch.crosshair.hide_lightgun");
	if (!strcasecmp(value, "always hide")) return TR("minarch.crosshair.always_hide");
	if (!strcasecmp(value, "always show")) return TR("minarch.crosshair.always_show");
	if (!strcasecmp(value, "hold start")) return TR("minarch.diag.hold_start");
	if (!strcasecmp(value, "start + a + b")) return TR("minarch.diag.start_ab");
	if (!strcasecmp(value, "hold start + a + b")) return TR("minarch.diag.hold_start_ab");
	if (!strcasecmp(value, "start + l + r")) return TR("minarch.diag.start_lr");
	if (!strcasecmp(value, "hold start + l + r")) return TR("minarch.diag.hold_start_lr");
	if (!strcasecmp(value, "hold select")) return TR("minarch.diag.hold_select");
	if (!strcasecmp(value, "select + a + b")) return TR("minarch.diag.select_ab");
	if (!strcasecmp(value, "hold select + a + b")) return TR("minarch.diag.hold_select_ab");
	if (!strcasecmp(value, "select + l + r")) return TR("minarch.diag.select_lr");
	if (!strcasecmp(value, "hold select + l + r")) return TR("minarch.diag.hold_select_lr");
	if (!strcasecmp(value, "use bios set in bios dipswitch")) return TR("minarch.neogeo.use_dipswitch");
	if (!strcasecmp(value, "mvs europe/asia (english)")) return TR("minarch.neogeo.mvs_eur");
	if (!strcasecmp(value, "mvs usa (english - censored)")) return TR("minarch.neogeo.mvs_usa");
	if (!strcasecmp(value, "mvs japan (japanese)")) return TR("minarch.neogeo.mvs_jap");
	if (!strcasecmp(value, "aes europe/asia (english)")) return TR("minarch.neogeo.aes_eur");
	if (!strcasecmp(value, "aes japan (japanese)")) return TR("minarch.neogeo.aes_jap");
	if (!strcasecmp(value, "unibios")) return TR("minarch.neogeo.unibios");
	if (!strcasecmp(value, "shared")) return TR("minarch.memcard.shared");
	if (!strcasecmp(value, "per-game")) return TR("minarch.memcard.per_game");
	if (!strcasecmp(value, "fixed")) return TR("minarch.frameskip.fixed");
	if (!strcasecmp(value, "auto")) return TR("common.auto");
	if (!strcasecmp(value, "manual")) return TR("minarch.frameskip.manual");
	if (!strcasecmp(value, "2-point 1st order")) return TR("minarch.interpolation.2point_1st");
	if (!strcasecmp(value, "4-point 3rd order")) return TR("minarch.interpolation.4point_3rd");
	
	// Atari 800/5200 specific
	if (!strcasecmp(value, "400/800 (os b)")) return TR("minarch.system.400_800_osb");
	if (!strcasecmp(value, "800xl (64k)")) return TR("minarch.system.800xl_64k");
	if (!strcasecmp(value, "130xe (128k)")) return TR("minarch.system.130xe_128k");
	if (!strcasecmp(value, "sio acceleration")) return TR("minarch.sio.acceleration");
	if (!strcasecmp(value, "boot from cassette")) return TR("minarch.boot.cassette");
	if (!strcasecmp(value, "hi-res artifacting")) return TR("minarch.hires.artifacting");
	if (!strcasecmp(value, "autodetect a5200 carttype")) return TR("minarch.autodetect.a5200_cart");
	if (!strcasecmp(value, "joy hack a5200 for robotron")) return TR("minarch.joyhack.robotron");
	if (!strcasecmp(value, "retroarch keyboard type")) return TR("minarch.keyboard.type");
	if (!strcasecmp(value, "poll")) return TR("minarch.keyboard.poll");
	if (!strcasecmp(value, "callback")) return TR("minarch.keyboard.callback");
	
	return value;
}

char** Minarch_buildCommonValueLabels(char** values) {
	int count = 0;
	while (values && values[count]) count++;
	char** labels = calloc(count + 1, sizeof(char*));
	for (int i = 0; i < count; i++) {
		const char* translated = Minarch_translateCommonValue(values[i]);
		labels[i] = strdup(translated ? translated : values[i]);
	}
	labels[count] = NULL;
	return labels;
}

static char** i18n_rewind_accel_labels = NULL;

void Minarch_initFrontendI18nOnce(void) {
	static int initialized = 0;
	if (initialized) return;
	initialized = 1;

	// NOTE: I18N_init() runs at the top of main(), before Config_load().
	static const char* onoff_keys[] = {"common.off", "common.on", NULL};
	static const char* scaling_keys[] = {"minarch.scaling.native", "minarch.scaling.aspect", "minarch.scaling.aspect_screen", "minarch.scaling.fullscreen", "minarch.scaling.cropped", NULL};
	static const char* resample_keys[] = {"minarch.resample.low", "minarch.resample.medium", "minarch.resample.high", "minarch.resample.max", NULL};
	static const char* ambient_keys[] = {"minarch.ambient.off", "minarch.ambient.all", "minarch.ambient.top", "minarch.ambient.fn", "minarch.ambient.lr", "minarch.ambient.top_lr", NULL};
	static const char* effect_keys[] = {"common.none", "minarch.effect.line", "minarch.effect.grid", NULL};
	static const char* overlay_keys[] = {"common.none", NULL};
	static const char* sync_ref_keys[] = {"common.auto", "minarch.sync_ref.screen", "minarch.sync_ref.native", NULL};
	// upstream order: Auto, Performance, Powersave
	static const char* overclock_keys[] = {"common.auto", "minarch.cpu.performance", "minarch.cpu.powersave", NULL};
	// For max FF, only translate the first entry (None).
	static const char* max_ff_keys[] = {"common.none", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	static const char* rewind_accel_keys[] = {"minarch.rewind.accel.1", "minarch.rewind.accel.2", "minarch.rewind.accel.4", "minarch.rewind.accel.8", "minarch.rewind.accel.12", NULL};

	i18n_onoff_labels = Minarch_buildI18nLabels(onoff_keys, onoff_labels);
	i18n_scaling_labels = Minarch_buildI18nLabels(scaling_keys, scaling_labels);
	i18n_resample_labels = Minarch_buildI18nLabels(resample_keys, resample_labels);
	i18n_ambient_labels = Minarch_buildI18nLabels(ambient_keys, ambient_labels);
	i18n_effect_labels = Minarch_buildI18nLabels(effect_keys, effect_labels);
	i18n_overlay_labels = Minarch_buildI18nLabels(overlay_keys, overlay_labels);
	i18n_sync_ref_labels = Minarch_buildI18nLabels(sync_ref_keys, sync_ref_labels);
	i18n_overclock_labels = Minarch_buildI18nLabels(overclock_keys, overclock_labels);
	i18n_max_ff_labels = Minarch_buildI18nLabels(max_ff_keys, max_ff_labels);
	i18n_rewind_accel_labels = Minarch_buildI18nLabels(rewind_accel_keys, rewind_compression_accel_labels);

	static const char* sharpness_keys[] = {"minarch.sharpness.nearest", "minarch.sharpness.linear", NULL};
	i18n_sharpness_labels = Minarch_buildI18nLabels(sharpness_keys, sharpness_labels);

	static const char* nrofshaders_keys[] = {"common.off", NULL, NULL, NULL, NULL};
	static const char* shupscale_keys[] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "minarch.shaders.upscale.screen", NULL};
	static const char* shscaletype_keys[] = {"minarch.shaders.scale_from.source", "minarch.shaders.scale_from.relative", NULL};

	i18n_nrofshaders_labels = Minarch_buildI18nLabels(nrofshaders_keys, nrofshaders_labels);
	i18n_shupscale_labels = Minarch_buildI18nLabels(shupscale_keys, shupscale_labels);
	i18n_shscaletype_labels = Minarch_buildI18nLabels(shscaletype_keys, shscaletype_labels);

	// Button labels for controls and shortcuts (must follow button_labels[] order)
	static const char* button_keys[] = {
		"common.none",
		"common.up", "common.down", "common.left", "common.right",
		"common.a", "common.b", "common.x", "common.y",
		"common.start", "common.select",
		"common.l1", "common.r1", "common.l2", "common.r2", "common.l3", "common.r3",
		"common.l4", "common.r4",
		"common.menu_up", "common.menu_down", "common.menu_left", "common.menu_right",
		"common.menu_a", "common.menu_b", "common.menu_x", "common.menu_y",
		"common.menu_start", "common.menu_select",
		"common.menu_l1", "common.menu_r1", "common.menu_l2", "common.menu_r2",
		"common.menu_l3", "common.menu_r3",
		"common.menu_l4", "common.menu_r4",
		NULL
	};
	i18n_button_labels = Minarch_buildI18nLabels(button_keys, button_labels);

	// Gamepad type labels
	static const char* gamepad_keys[] = {"minarch.gamepad.standard", "minarch.gamepad.dualshock", NULL};
	i18n_gamepad_labels = Minarch_buildI18nLabels(gamepad_keys, gamepad_labels);
}

// Apply translated names/labels to the option tables. Called from Config_load()
// after the upstream label truncation (overscan/ambient) so the display labels
// stay in lock-step with option->values.
static void Minarch_applyFrontendI18n(void) {
	Minarch_initFrontendI18nOnce();

	if (scaling_labels[4] == NULL) i18n_scaling_labels[4] = NULL;
	if (ambient_labels[2] == NULL) {
		i18n_ambient_labels[1] = (char*)TR("common.on");
		i18n_ambient_labels[2] = NULL;
	}

	Option* fe = config.frontend.options;
	fe[FE_OPT_SCALING].name = (char*)TR("minarch.frontend.screen_scaling");
	fe[FE_OPT_SCALING].labels = i18n_scaling_labels;
	fe[FE_OPT_RESAMPLING].name = (char*)TR("minarch.frontend.audio_resampling_quality");
	fe[FE_OPT_RESAMPLING].labels = i18n_resample_labels;
	fe[FE_OPT_AMBIENT].name = (char*)TR("minarch.frontend.ambient_mode");
	fe[FE_OPT_AMBIENT].labels = i18n_ambient_labels;
	fe[FE_OPT_EFFECT].name = (char*)TR("minarch.frontend.screen_effect");
	fe[FE_OPT_EFFECT].labels = i18n_effect_labels;
	fe[FE_OPT_OVERLAY].name = (char*)TR("minarch.frontend.overlay");
	fe[FE_OPT_OVERLAY].labels = i18n_overlay_labels;
	fe[FE_OPT_SCREENX].name = (char*)TR("minarch.frontend.offset_screen_x");
	fe[FE_OPT_SCREENY].name = (char*)TR("minarch.frontend.offset_screen_y");
	fe[FE_OPT_SHARPNESS].name = (char*)TR("minarch.frontend.screen_sharpness");
	fe[FE_OPT_SHARPNESS].labels = i18n_sharpness_labels;
	fe[FE_OPT_SYNC_REFERENCE].name = (char*)TR("minarch.frontend.core_sync");
	fe[FE_OPT_SYNC_REFERENCE].labels = i18n_sync_ref_labels;
	fe[FE_OPT_OVERCLOCK].name = (char*)TR("minarch.frontend.cpu_speed");
	fe[FE_OPT_OVERCLOCK].labels = i18n_overclock_labels;
	fe[FE_OPT_DEBUG].name = (char*)TR("minarch.frontend.debug_hud");
	fe[FE_OPT_DEBUG].labels = i18n_onoff_labels;
	fe[FE_OPT_MAXFF].name = (char*)TR("minarch.frontend.max_ff_speed");
	fe[FE_OPT_MAXFF].labels = i18n_max_ff_labels;
	fe[FE_OPT_FF_AUDIO].name = (char*)TR("minarch.frontend.ff_audio");
	fe[FE_OPT_FF_AUDIO].labels = i18n_onoff_labels;

	fe[FE_OPT_REWIND_ENABLE].name = (char*)TR("minarch.frontend.rewind");
	fe[FE_OPT_REWIND_ENABLE].desc = (char*)TR("minarch.frontend.rewind.desc");
	fe[FE_OPT_REWIND_ENABLE].labels = i18n_onoff_labels;
	fe[FE_OPT_REWIND_BUFFER].name = (char*)TR("minarch.frontend.rewind_buffer");
	fe[FE_OPT_REWIND_BUFFER].desc = (char*)TR("minarch.frontend.rewind_buffer.desc");
	fe[FE_OPT_REWIND_GRANULARITY].name = (char*)TR("minarch.frontend.rewind_interval");
	fe[FE_OPT_REWIND_GRANULARITY].desc = (char*)TR("minarch.frontend.rewind_interval.desc");
	fe[FE_OPT_REWIND_COMPRESSION].name = (char*)TR("minarch.frontend.rewind_compression");
	fe[FE_OPT_REWIND_COMPRESSION].desc = (char*)TR("minarch.frontend.rewind_compression.desc");
	fe[FE_OPT_REWIND_COMPRESSION].labels = i18n_onoff_labels;
	fe[FE_OPT_REWIND_COMPRESSION_ACCEL].name = (char*)TR("minarch.frontend.rewind_compression_speed");
	fe[FE_OPT_REWIND_COMPRESSION_ACCEL].desc = (char*)TR("minarch.frontend.rewind_compression_speed.desc");
	fe[FE_OPT_REWIND_COMPRESSION_ACCEL].labels = i18n_rewind_accel_labels;
	fe[FE_OPT_REWIND_AUDIO].name = (char*)TR("minarch.frontend.rewind_audio");
	fe[FE_OPT_REWIND_AUDIO].desc = (char*)TR("minarch.frontend.rewind_audio.desc");
	fe[FE_OPT_REWIND_AUDIO].labels = i18n_onoff_labels;

	// Translate shader menu option names/descriptions and display labels (keep option->values stable).
	config.shaders.options[SH_EXTRASETTINGS].name = (char*)TR("minarch.shaders.optional_settings");
	config.shaders.options[SH_EXTRASETTINGS].desc = (char*)TR("minarch.shaders.optional_settings.desc");

	config.shaders.options[SH_SHADERS_PRESET].name = (char*)TR("minarch.shaders.preset");
	config.shaders.options[SH_SHADERS_PRESET].desc = (char*)TR("minarch.shaders.preset.desc");

	config.shaders.options[SH_NROFSHADERS].name = (char*)TR("minarch.shaders.count");
	config.shaders.options[SH_NROFSHADERS].desc = (char*)TR("minarch.shaders.count.desc");
	config.shaders.options[SH_NROFSHADERS].labels = i18n_nrofshaders_labels;

	config.shaders.options[SH_SHADER1].name = (char*)TR("minarch.shaders.shader1");
	config.shaders.options[SH_SHADER1].desc = (char*)TR("minarch.shaders.shader.desc");
	config.shaders.options[SH_SHADER1_FILTER].name = (char*)TR("minarch.shaders.shader1_filter");
	config.shaders.options[SH_SHADER1_FILTER].desc = (char*)TR("minarch.shaders.filter.desc");
	config.shaders.options[SH_SRCTYPE1].name = (char*)TR("minarch.shaders.shader1_source_type");
	config.shaders.options[SH_SRCTYPE1].desc = (char*)TR("minarch.shaders.source_type.desc");
	config.shaders.options[SH_SRCTYPE1].labels = i18n_shscaletype_labels;
	config.shaders.options[SH_SCALETYPE1].name = (char*)TR("minarch.shaders.shader1_texture_type");
	config.shaders.options[SH_SCALETYPE1].desc = (char*)TR("minarch.shaders.texture_type.desc");
	config.shaders.options[SH_SCALETYPE1].labels = i18n_shscaletype_labels;
	config.shaders.options[SH_UPSCALE1].name = (char*)TR("minarch.shaders.shader1_scale");
	config.shaders.options[SH_UPSCALE1].desc = (char*)TR("minarch.shaders.scale.desc");
	config.shaders.options[SH_UPSCALE1].labels = i18n_shupscale_labels;

	config.shaders.options[SH_SHADER2].name = (char*)TR("minarch.shaders.shader2");
	config.shaders.options[SH_SHADER2].desc = (char*)TR("minarch.shaders.shader.desc");
	config.shaders.options[SH_SHADER2_FILTER].name = (char*)TR("minarch.shaders.shader2_filter");
	config.shaders.options[SH_SHADER2_FILTER].desc = (char*)TR("minarch.shaders.filter.desc");
	config.shaders.options[SH_SRCTYPE2].name = (char*)TR("minarch.shaders.shader2_source_type");
	config.shaders.options[SH_SRCTYPE2].desc = (char*)TR("minarch.shaders.source_type.desc");
	config.shaders.options[SH_SRCTYPE2].labels = i18n_shscaletype_labels;
	config.shaders.options[SH_SCALETYPE2].name = (char*)TR("minarch.shaders.shader2_texture_type");
	config.shaders.options[SH_SCALETYPE2].desc = (char*)TR("minarch.shaders.texture_type.desc");
	config.shaders.options[SH_SCALETYPE2].labels = i18n_shscaletype_labels;
	config.shaders.options[SH_UPSCALE2].name = (char*)TR("minarch.shaders.shader2_scale");
	config.shaders.options[SH_UPSCALE2].desc = (char*)TR("minarch.shaders.scale.desc");
	config.shaders.options[SH_UPSCALE2].labels = i18n_shupscale_labels;

	config.shaders.options[SH_SHADER3].name = (char*)TR("minarch.shaders.shader3");
	config.shaders.options[SH_SHADER3].desc = (char*)TR("minarch.shaders.shader.desc");
	config.shaders.options[SH_SHADER3_FILTER].name = (char*)TR("minarch.shaders.shader3_filter");
	config.shaders.options[SH_SHADER3_FILTER].desc = (char*)TR("minarch.shaders.filter.desc");
	config.shaders.options[SH_SRCTYPE3].name = (char*)TR("minarch.shaders.shader3_source_type");
	config.shaders.options[SH_SRCTYPE3].desc = (char*)TR("minarch.shaders.source_type.desc");
	config.shaders.options[SH_SRCTYPE3].labels = i18n_shscaletype_labels;
	config.shaders.options[SH_SCALETYPE3].name = (char*)TR("minarch.shaders.shader3_texture_type");
	config.shaders.options[SH_SCALETYPE3].desc = (char*)TR("minarch.shaders.texture_type.desc");
	config.shaders.options[SH_SCALETYPE3].labels = i18n_shscaletype_labels;
	config.shaders.options[SH_UPSCALE3].name = (char*)TR("minarch.shaders.shader3_scale");
	config.shaders.options[SH_UPSCALE3].desc = (char*)TR("minarch.shaders.scale.desc");
	config.shaders.options[SH_UPSCALE3].labels = i18n_shupscale_labels;
}

// CONFIG_NONE, CONFIG_CONSOLE, CONFIG_GAME defined in minarch_internal.h

char* getScreenScalingDesc(void) {
	if (GFX_supportsOverscan()) {
		return "Native uses integer scaling. Aspect uses core\nreported aspect ratio. Aspect screen uses screen\naspect ratio. Fullscreen has non-square pixels.\nCropped is integer scaled then cropped.";
	}
	else {
		return "Native uses integer scaling.\nAspect uses core reported aspect ratio.\nAspect screen uses screen aspect ratio.\nFullscreen has non-square pixels.";
	}
}
int getScreenScalingCount(void) {
	return GFX_supportsOverscan() ? 5 : 4;
}

// The ambient zone list is the Brick layout: All, Top bar, FN keys, L/R.
// Devices with two lights only have the pair GFX_setAmbientColor() writes for
// mode 1, so offer that as a plain on/off rather than four options that mostly
// do nothing. No value remapping needed: index 1 is already mode 1. The menu
// walks labels until NULL, so truncating that is what limits the choices.
static void configureAmbientOption(void) {
	Option* ambient_option = &config.frontend.options[FE_OPT_AMBIENT];
	int lights = LEDS_getCount();
	if (lights == 0) {
		ambient_option->lock = 1; // nothing to light up, keep it out of the menu
	}
	else if (lights <= 2) {
		ambient_labels[1] = "On";
		ambient_labels[2] = NULL;
		ambient_option->count = 2;
	}
}
	

struct Config config = {
	.frontend = { // (OptionList)
		.count = FE_OPT_COUNT,
		.options = (Option[]){
			[FE_OPT_SCALING] = {
				.key	= "minarch_screen_scaling", 
				.name	= "Screen Scaling",
				.desc	= NULL, // will call getScreenScalingDesc()
				.default_value = 1,
				.value = 1,
				.count = 3, // will call getScreenScalingCount()
				.values = scaling_labels,
				.labels = scaling_labels,
			},
			[FE_OPT_RESAMPLING] = {
				.key	= "minarch__resampling_quality", 
				.name	= "Audio Resampling Quality",
				.desc	= "Resampling quality higher takes more CPU",
				.default_value = 2,
				.value = 2,
				.count = 4,
				.values = resample_labels,
				.labels = resample_labels,
			},
			[FE_OPT_AMBIENT] = {
				.key	= "minarch_ambient", 
				.name	= "Ambient Mode",
				.desc	= "Makes your leds follow on screen colors",
				.default_value = 0,
				.value = 0,
				.count = 6,
				.values = ambient_labels,
				.labels = ambient_labels,
			},
			[FE_OPT_EFFECT] = {
				.key	= "minarch_screen_effect",
				.name	= "Screen Effect",
				.desc	= "Grid simulates an LCD grid.\nLine simulates CRT scanlines.\nEffects usually look best at native scaling.",
				.default_value = 0,
				.value = 0,
				.count = 3,
				.values = effect_labels,
				.labels = effect_labels,
			},
			[FE_OPT_OVERLAY] = {
				.key	= "minarch_overlay",
				.name	= "Overlay",
				.desc	= "Choose a custom overlay png from the Overlays folder",
				.default_value = 0,
				.value = 0,
				.count = 1,
				.values = overlay_labels,
				.labels = overlay_labels,
			},
			[FE_OPT_SCREENX] = {
				.key	= "minarch_screen_offsetx",
				.name	= "Offset screen X",
				.desc	= "Offset X pixels",
				.default_value = 64,
				.value = 64,
				.count = 129,
				.values = offset_labels,
				.labels = offset_labels,
			},
			[FE_OPT_SCREENY] = {
				.key	= "minarch_screen_offsety",
				.name	= "Offset screen Y",
				.desc	= "Offset Y pixels",
				.default_value = 64,
				.value = 64,
				.count = 129,
				.values = offset_labels,
				.labels = offset_labels,
			},
			[FE_OPT_SHARPNESS] = {
				// 	.key	= "minarch_screen_sharpness",
				.key	= "minarch_scale_filter",
				.name	= "Screen Sharpness",
				.desc	= "LINEAR smooths lines, but works better when final image is at higher resolution, so either core that outputs higher resolution or upscaling with shaders",
				.default_value = 1,
				.value = 1,
				.count = 3,
				.values = sharpness_labels,
				.labels = sharpness_labels,
			},
			[FE_OPT_SYNC_REFERENCE] = {
				.key	= "minarch_sync_reference",
				.name	= "Core Sync",
				.desc	= "Choose what should be used as a\nreference for the frame rate.\n\"Native\" uses the emulator frame rate,\n\"Screen\" uses the frame rate of the screen.",
				.default_value = SYNC_SRC_AUTO,
				.value = SYNC_SRC_AUTO,
				.count = 3,
				.values = sync_ref_labels,
				.labels = sync_ref_labels,
			},
			[FE_OPT_OVERCLOCK] = {
				.key	= "minarch_cpu_speed",
				.name	= "CPU Speed",
				.desc	= "Choose how the CPU scales.\nAuto is recommended for most users.",
				.default_value = 0,
				.value = 0,
				.count = 3,
				.values = overclock_labels,
				.labels = overclock_labels,
			},
			[FE_OPT_DEBUG] = {
				.key	= "minarch_debug_hud",
				.name	= "Debug HUD",
				.desc	= "Show frames per second, cpu load,\nresolution, and scaler information.",
				.default_value = 0,
				.value = 0,
				.count = 2,
				.values = onoff_labels,
				.labels = onoff_labels,
			},
			[FE_OPT_MAXFF] = {
				.key	= "minarch_max_ff_speed",
				.name	= "Max FF Speed",
				.desc	= "Fast forward will not exceed the\nselected speed (but may be less\ndepending on game and emulator).",
				.default_value = 3, // 4x
				.value = 3, // 4x
				.count = 8,
				.values = max_ff_labels,
				.labels = max_ff_labels,
			},
			[FE_OPT_FF_AUDIO] = {
				.key	= "minarch__ff_audio", 
				.name	= "Fast forward audio",
				.desc	= "Play or mute audio when fast forwarding.",
				.default_value = 0,
				.value = 0,
				.count = 2,
				.values = onoff_labels,
				.labels = onoff_labels,
			},
			[FE_OPT_REWIND_ENABLE] = {
				.key	= "minarch_rewind_enable",
				.name	= "Rewind",
				.desc	= "Enable in-memory rewind buffer.\nMust set a shortcut to access rewind during gameplay.\nUses extra CPU and memory.",
				.default_value = MINARCH_DEFAULT_REWIND_ENABLE ? 1 : 0,
				.value = MINARCH_DEFAULT_REWIND_ENABLE ? 1 : 0,
				.count = 2,
				.values = rewind_enable_labels,
				.labels = rewind_enable_labels,
			},
			[FE_OPT_REWIND_BUFFER] = {
				.key	= "minarch_rewind_buffer_mb",
				.name	= "Rewind Buffer (MB)",
				.desc	= "Memory reserved for rewind snapshots.\nIncrease for longer rewind times.",
				.default_value = 3, // 64MB
				.value = 3,
				.count = 6,
				.values = rewind_buffer_labels,
				.labels = rewind_buffer_labels,
			},
			[FE_OPT_REWIND_GRANULARITY] = {
				.key	= "minarch_rewind_granularity",
				.name	= "Rewind Interval",
				.desc	= "Interval between rewind snapshots.\nShorter intervals improve smoothness during rewind,\nbut increase CPU and memory usage.",
				.default_value = 0, // 16ms
				.value = 0,
				.count = 12,
				.values = rewind_granularity_values,
				.labels = rewind_granularity_labels,
			},
			[FE_OPT_REWIND_COMPRESSION] = {
				.key	= "minarch_rewind_compression",
				.name	= "Rewind Compression",
				.desc	= "Compress rewind snapshots to save memory at the cost of CPU.",
				.default_value = 1,
				.value = 1,
				.count = 2,
				.values = onoff_labels,
				.labels = onoff_labels,
			},
			[FE_OPT_REWIND_COMPRESSION_ACCEL] = {
				.key	= "minarch_rewind_compression_speed",
				.name	= "Rewind Compression Speed",
				.desc	= "LZ4 acceleration used for rewind snapshots.\nLower values compress more but use more CPU.",
				.default_value = 1, // value 2
				.value = 1,
				.count = 5,
				.values = rewind_compression_accel_values,
				.labels = rewind_compression_accel_labels,
			},
			[FE_OPT_REWIND_AUDIO] = {
				.key	= "minarch_rewind_audio",
				.name	= "Rewind audio",
				.desc	= "Play or mute audio when rewinding.",
				.default_value = MINARCH_DEFAULT_REWIND_AUDIO ? 1 : 0,
				.value = MINARCH_DEFAULT_REWIND_AUDIO ? 1 : 0,
				.count = 2,
				.values = onoff_labels,
				.labels = onoff_labels,
			},
			[FE_OPT_COUNT] = {NULL}
		}
	},
	.core = { // (OptionList)
		.count = 0,
		.options = (Option[]){
			{NULL},
		},
	},
	.shaders = { // (OptionList)
		.count = 18,
		.options = (Option[]){
			[SH_EXTRASETTINGS] = {
				.key	= "minarch_shaders_settings", 
				.name	= "Optional Shaders Settings",
				.desc	= "If shaders have extra settings they will show up in this settings menu",
				.default_value = 1,
				.value = 1,
				.count = 0,
				.values = NULL,
				.labels = NULL,
			},
			[SH_SHADERS_PRESET] = {
				.key	= "minarch_shaders_preset", 
				.name	= "Shader / Emulator Settings Preset",
				.desc	= "Load a premade shaders/emulators config.\nTo try out a preset, exit the game without saving settings!",
				.default_value = 1,
				.value = 1,
				.count = 0,
				.values = NULL,
				.labels = NULL,
			},
			[SH_NROFSHADERS] = {
				.key	= "minarch_nrofshaders", 
				.name	= "Number of Shaders",
				.desc	= "Number of shaders 1 to 3",
				.default_value = 0,
				.value = 0,
				.count = 4,
				.values = nrofshaders_labels,
				.labels = nrofshaders_labels,
			},
			
			[SH_SHADER1] = {
				.key	= "minarch_shader1", 
				.name	= "Shader 1",
				.desc	= "Shader 1 program to run",
				.default_value = 1,
				.value = 1,
				.count = 0,
				.values = NULL,
				.labels = NULL,
			},
			[SH_SHADER1_FILTER] = {
				.key	= "minarch_shader1_filter", 
				.name	= "Shader 1 Filter",
				.desc	= "Method of upscaling, NEAREST or LINEAR",
				.default_value = 1,
				.value = 1,
				.count = 2,
				.values = shfilter_labels,
				.labels = shfilter_labels,
			},
			[SH_SRCTYPE1] = {
				.key	= "minarch_shader1_srctype", 
				.name	= "Shader 1 Source type",
				.desc	= "This will choose resolution source to scale from",
				.default_value = 0,
				.value = 0,
				.count = 2,
				.values = shscaletype_labels,
				.labels = shscaletype_labels,
			},
			[SH_SCALETYPE1] = {
				.key	= "minarch_shader1_scaletype", 
				.name	= "Shader 1 Texture Type",
				.desc	= "This will choose resolution source to scale from",
				.default_value = 1,
				.value = 1,
				.count = 2,
				.values = shscaletype_labels,
				.labels = shscaletype_labels,
			},
			[SH_UPSCALE1] = {
				.key	= "minarch_shader1_upscale", 
				.name	= "Shader 1 Scale",
				.desc	= "This will scale images x times,\nscreen scales to screens resolution (can hit performance)",
				.default_value = 1,
				.value = 1,
				.count = 9,
				.values = shupscale_labels,
				.labels = shupscale_labels,
			},
			[SH_SHADER2] = {
				.key	= "minarch_shader2", 
				.name	= "Shader 2",
				.desc	= "Shader 2 program to run",
				.default_value = 0,
				.value = 0,
				.count = 0,
				.values = NULL,
				.labels = NULL,

			},
			[SH_SHADER2_FILTER] = {
				.key	= "minarch_shader2_filter", 
				.name	= "Shader 2 Filter",
				.desc	= "Method of upscaling, NEAREST or LINEAR",
				.default_value = 0,
				.value = 0,
				.count = 2,
				.values = shfilter_labels,
				.labels = shfilter_labels,
			},
			[SH_SRCTYPE2] = {
				.key	= "minarch_shader2_srctype", 
				.name	= "Shader 2 Source type",
				.desc	= "This will choose resolution source to scale from",
				.default_value = 0,
				.value = 0,
				.count = 2,
				.values = shscaletype_labels,
				.labels = shscaletype_labels,
			},
			[SH_SCALETYPE2] = {
				.key	= "minarch_shader2_scaletype", 
				.name	= "Shader 2 Texture Type",
				.desc	= "This will choose resolution source to scale from",
				.default_value = 1,
				.value = 1,
				.count = 2,
				.values = shscaletype_labels,
				.labels = shscaletype_labels,
			},
			[SH_UPSCALE2] = {
				.key	= "minarch_shader2_upscale", 
				.name	= "Shader 2 Scale",
				.desc	= "This will scale images x times,\nscreen scales to screens resolution (can hit performance)",
				.default_value = 0,
				.value = 0,
				.count = 9,
				.values = shupscale_labels,
				.labels = shupscale_labels,
			},
			[SH_SHADER3] = {
				.key	= "minarch_shader3", 
				.name	= "Shader 3",
				.desc	= "Shader 3 program to run",
				.default_value = 2,
				.value = 2,
				.count = 0,
				.values = NULL,
				.labels = NULL,

			},
			[SH_SHADER3_FILTER] = {
				.key	= "minarch_shader3_filter", 
				.name	= "Shader 3 Filter",
				.desc	= "Method of upscaling, NEAREST or LINEAR",
				.default_value = 0,
				.value = 0,
				.count = 2,
				.values = shfilter_labels,
				.labels = shfilter_labels,
			},
			[SH_SRCTYPE3] = {
				.key	= "minarch_shader3_srctype", 
				.name	= "Shader 3 Source type",
				.desc	= "This will choose resolution source to scale from",
				.default_value = 0,
				.value = 0,
				.count = 2,
				.values = shscaletype_labels,
				.labels = shscaletype_labels,
			},
			[SH_SCALETYPE3] = {
				.key	= "minarch_shader3_scaletype", 
				.name	= "Shader 3 Texture Type",
				.desc	= "This will choose resolution source to scale from",
				.default_value = 1,
				.value = 1,
				.count = 2,
				.values = shscaletype_labels,
				.labels = shscaletype_labels,
			},
			[SH_UPSCALE3] = {
				.key	= "minarch_shader3_upscale", 
				.name	= "Shader 3 Scale",
				.desc	= "This will scale images x times,\nscreen scales to screens resolution (can hit performance)",
				.default_value = 0,
				.value = 0,
				.count = 9,
				.values = shupscale_labels,
				.labels = shupscale_labels,
			},
			{NULL}
		},
	},
	.shaderpragmas = {{
		.count = 0,
		.options = NULL,
	}},
	.controls = default_button_mapping,
	.shortcuts = (ButtonMapping[]){
		[SHORTCUT_SAVE_STATE]			= {"Save State",		-1, BTN_ID_NONE, 0},
		[SHORTCUT_LOAD_STATE]			= {"Load State",		-1, BTN_ID_NONE, 0},
		[SHORTCUT_RESET_GAME]			= {"Reset Game",		-1, BTN_ID_NONE, 0},
		[SHORTCUT_SAVE_QUIT]			= {"Save & Quit",		-1, BTN_ID_NONE, 0},
		[SHORTCUT_CYCLE_SCALE]			= {"Cycle Scaling",		-1, BTN_ID_NONE, 0},
		[SHORTCUT_CYCLE_EFFECT]			= {"Cycle Effect",		-1, BTN_ID_NONE, 0},
		[SHORTCUT_TOGGLE_FF]			= {"Toggle FF",			-1, BTN_ID_NONE, 0},
		[SHORTCUT_HOLD_FF]				= {"Hold FF",			-1, BTN_ID_NONE, 0},
		[SHORTCUT_TOGGLE_REWIND]		= {"Toggle Rewind",		-1, BTN_ID_NONE, 0},
		[SHORTCUT_HOLD_REWIND]			= {"Hold Rewind",		-1, BTN_ID_NONE, 0},
		[SHORTCUT_GAMESWITCHER]			= {"Game Switcher",		-1, BTN_ID_NONE, 0},
		[SHORTCUT_SCREENSHOT]           = {"Screenshot",        -1, BTN_ID_NONE, 0},
		// Trimui only
		[SHORTCUT_TOGGLE_TURBO_A]		= {"Toggle Turbo A",	-1, BTN_ID_NONE, 0},
		[SHORTCUT_TOGGLE_TURBO_B]		= {"Toggle Turbo B",	-1, BTN_ID_NONE, 0},
		[SHORTCUT_TOGGLE_TURBO_X]		= {"Toggle Turbo X",	-1, BTN_ID_NONE, 0},
		[SHORTCUT_TOGGLE_TURBO_Y]		= {"Toggle Turbo Y",	-1, BTN_ID_NONE, 0},
		[SHORTCUT_TOGGLE_TURBO_L]		= {"Toggle Turbo L",	-1, BTN_ID_NONE, 0},
		[SHORTCUT_TOGGLE_TURBO_L2]		= {"Toggle Turbo L2",	-1, BTN_ID_NONE, 0},
		[SHORTCUT_TOGGLE_TURBO_R]		= {"Toggle Turbo R",	-1, BTN_ID_NONE, 0},
		[SHORTCUT_TOGGLE_TURBO_R2]		= {"Toggle Turbo R2",	-1, BTN_ID_NONE, 0},
		// -----
		{NULL}
	},
};


