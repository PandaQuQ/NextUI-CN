#ifndef I18N_H
#define I18N_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef enum {
    LANG_EN_US = 0,  // English
    LANG_ZH_CN = 1,  // Simplified Chinese
    LANG_JA_JP = 2,  // Japanese
    LANG_KO_KR = 3,  // Korean
    LANG_COUNT
} Language;

// Language change callback function type
typedef void (*LanguageChangeCallback)(void);

// Initialize internationalization system
void I18N_init(void);

// Set current language
void I18N_setLanguage(Language lang);

// Get current language
Language I18N_getCurrentLanguage(void);

// Get translated text
const char* I18N_get(const char* key);

// Get language name
const char* I18N_getLanguageName(Language lang);

// Register language change callback
void I18N_registerCallback(LanguageChangeCallback callback);

// Unregister language change callback
void I18N_unregisterCallback(LanguageChangeCallback callback);

// Release resources
void I18N_cleanup(void);

// Convenience macro definition
#define _(key) I18N_get(key)

#ifdef __cplusplus
}
#endif

#endif // I18N_H
