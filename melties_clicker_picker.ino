// Copyright (c) 2025 YMATYT Holdings, LLC
//
// Meltie's clicker picker
#include <Adafruit_NeoPixel.h>

#include <React_Generic.h>
#include <React_Generic.hpp>

#include <Adafruit_CircuitPlayground.h>


#include <React_Generic.h>

using namespace react_generic;

React_Generic *app = nullptr;
React_Generic *new_app = nullptr;

// const auto NEOPIXEL_PIN = CPLAY_NEOPIXELPIN;
// const auto PIXEL_COUNT = 10;
const auto NEOPIXEL_PIN = 10;
const auto PIXEL_COUNT = 20;

const auto NUM_CHOICES = 16;

Adafruit_NeoPixel *strip;
Adafruit_NeoPixel *strip2;

class Spin {
  public:
    Spin(int num_choices);
    void render_spin(Adafruit_NeoPixel *strip);
    int choice;
    int num_choices;
};

Spin::Spin(int num_choices): num_choices(num_choices) {
  this->choice = random(num_choices);
  Serial.print("Random selection: ");
  Serial.println(this->choice);
}

uint32_t random_color(Adafruit_NeoPixel *strip) {
    uint8_t r = random(100);
    uint8_t g = random(100);
    uint8_t b = random(100);
    if ((r < 5) && (g < 5) && (b < 5)) {
      r = 15;
    }
    return strip->Color(r, g, b);
}

void Spin::render_spin(Adafruit_NeoPixel *strip) {
  strip->clear();
  strip->show();
  strip->setBrightness(50);

  auto steps = max(min(15, random(2 * num_choices)), 5);

  for (int i=0; i<steps; i++) {
    auto random_pixel = random(PIXEL_COUNT);
    Serial.println(i);

    strip->clear();
    strip->setPixelColor(random_pixel, random_color(strip));
    strip->show();
    delay(100);
  }
  
  strip->clear();
  strip->setPixelColor(this->choice+PIXEL_COUNT-num_choices, random_color(strip));
  strip->show();
}

void setup_spin() {
  Serial.println("Starting a spin...");
  new_app = new React_Generic(false);
  
  new_app->onTick([]() {
    auto num_choices = min(NUM_CHOICES, PIXEL_COUNT);
    if (num_choices < NUM_CHOICES) {
      Serial.println("Warning, # of choices exceeds # of pixels. Will only use # of pixels.");
    }
    Spin spin(num_choices);
    spin.render_spin(strip);

    setup_waiting();
  });
}

void setup_waiting() {
  Serial.println("setting up waiting...");
  new_app = new React_Generic(false);

  new_app->onRepeat(1000, []() {
      static bool state = false;
      digitalWrite(CPLAY_REDLED, state = !state);
  });

  Serial.println("  interrupt setup...");
  new_app->onInterrupt(CPLAY_LEFTBUTTON, RISING, []() {
    setup_spin();
  });
  new_app->onInterrupt(CPLAY_RIGHTBUTTON, RISING, []() {
    Serial.println("HELLO RIGHT");
  });
  pinMode(6, INPUT_PULLDOWN);
  new_app->onInterrupt(6, RISING, []() {
    static auto last_action_time = 0;
    auto now = millis();
    if (now - last_action_time > 250) {
      Serial.println("HELLO Morse");
      last_action_time = now;
      setup_spin();
    }
  });

}

void setup() {
  CircuitPlayground.begin(50);
  randomSeed(CircuitPlayground.soundSensor());
  strip = new Adafruit_NeoPixel(PIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
  strip->begin();
  strip->clear();
  strip->setPixelColor(PIXEL_COUNT - NUM_CHOICES, strip->Color(255, 0, 0));
  strip->show();

  delay(2000);
  Serial.begin(115200);
  Serial.println("Setting up spinner...");

  setup_waiting();
  app = new_app;
  new_app = nullptr;
}

void loop() {
  if (new_app != nullptr) {
    noInterrupts();
    delete app;
    app = new_app;
    new_app = nullptr;
    interrupts();
  }
  app->tick();
}
