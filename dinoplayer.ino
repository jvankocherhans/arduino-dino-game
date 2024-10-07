#include "dino1.h"
#include "dino2.h"
#include "dino3.h"
#include "dino4.h"
#include "dino5.h"
#include "Arduino_H7_Video.h"
#include "ArduinoGraphics.h"
#include "Arduino_GigaDisplayTouch.h"
#include "Arduino_GigaDisplay_GFX.h"

Arduino_GigaDisplayTouch touchDetector;

struct Animation {
    int frames;
    int width;
    int height;
    const uint16_t PROGMEM* data;
};

Animation animations[] = {
    {dino1_frames, dino1_animation_width, dino1_animation_height, (const uint16_t*)dino1_animation_frames},
    {dino2_frames, dino2_animation_width, dino2_animation_height, (const uint16_t*)dino2_animation_frames},
    {dino3_frames, dino3_animation_width, dino3_animation_height, (const uint16_t*)dino3_animation_frames},
    {dino4_frames, dino4_animation_width, dino4_animation_height, (const uint16_t*)dino4_animation_frames},
    {dino5_frames, dino5_animation_width, dino5_animation_height, (const uint16_t*)dino5_animation_frames}
};

int currentAnimation = 0;
unsigned long lastSwitchTime = 0;
const unsigned long autoSwitchInterval = 8000;
const unsigned long holdTimeThreshold = 3000;

Arduino_H7_Video Display(800, 480, GigaDisplayShield);
#define ENCODING CM_RGB565

bool tapMode = false;
bool touching = false;
unsigned long touchStartTime = 0;
bool messageVisible = false;
unsigned long messageStartTime = 0;
const unsigned long messageDuration = 3000;

void displayMessage(const char* message) {
    Display.beginDraw();
    Display.clear();
    Display.background(0, 0, 255);
    Display.endDraw();

    messageVisible = true;
    messageStartTime = millis();
    delay(messageDuration);
    
    Display.beginDraw();
    Display.clear();
    Display.endDraw();
    messageVisible = false;
}

void setup() {
    Display.begin();
    Display.beginDraw();
    Display.background(0, 0, 0);
    Display.clear();
    Display.endDraw();

    if (!touchDetector.begin()) {
        while (1) {
            Serial.println("Touch controller init - FAILED");
        }
    }

    displayMessage("Tap Mode activated");
}

void loop() {
    unsigned long currentTime = millis();

    if (messageVisible) {
        return;
    }

    GDTpoint_t points[5];
    uint8_t contacts = touchDetector.getTouchPoints(points);

    if (contacts > 0) {
        if (!touching) {
            touching = true;
            touchStartTime = currentTime;
        } else if (currentTime - touchStartTime >= holdTimeThreshold) {
            tapMode = !tapMode;
            touching = false;

            if (tapMode) {
                displayMessage("Tap Mode activated");
            } else {
                displayMessage("Automatic Mode activated");
            }

            lastSwitchTime = currentTime;
        }
    } else {
        if (touching) {
            touching = false;
            if (currentTime - touchStartTime < holdTimeThreshold && tapMode) {
                currentAnimation = (currentAnimation + 1) % (sizeof(animations) / sizeof(animations[0]));
                lastSwitchTime = currentTime;
            }
        }
    }

    if (!tapMode && currentTime - lastSwitchTime >= autoSwitchInterval) {
        lastSwitchTime = currentTime;
        currentAnimation = (currentAnimation + 1) % (sizeof(animations) / sizeof(animations[0]));
    }

    Animation anim = animations[currentAnimation];

    for (int i = 0; i < anim.frames; i++) {
        Display.beginDraw();
        Image img(ENCODING, anim.data + (i * anim.width * anim.height), anim.width, anim.height);
        Display.image(img, ((Display.width() / 2) - (anim.width / 2)), ((Display.height() / 2) - (anim.height / 2)));
        Display.endDraw();
        delay(150);
    }
}
