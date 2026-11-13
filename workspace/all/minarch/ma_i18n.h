#ifndef MA_I18N_H
#define MA_I18N_H

// NextUI-CN: minarch display-label translation helpers (defined in ma_config.c)
#include "i18n.h"

extern char** i18n_button_labels;
extern char** i18n_gamepad_labels;

const char* Minarch_trOr(const char* key, const char* fallback);
const char* Minarch_translateCategoryLabel(const char* label);
char** Minarch_buildCommonValueLabels(char** values);
void Minarch_initFrontendI18nOnce(void);
// Display-only translation for shortcut names (the English name stays the config key).
const char* Minarch_trShortcutName(const char* name);

#endif
