// components/ui/onboarding.h
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

// Function to show the onboarding screen
void onboarding_show();

// Function to check if the onboarding screen is showing
bool onboarding_is_showing();

// Function to hide the onboarding screen
void onboarding_hide();

#ifdef __cplusplus
}
#endif
