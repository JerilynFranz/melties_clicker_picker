// Copyright (c) 2025 YMATYT Holdings, LLC
//
// Meltie's clicker picker
// This Arduino sketch uses a Circuit Playground board and an attached NeoPixel strip
// to randomly select an option, indicated by lighting up a specific LED after an animation.
// It uses buttons or an external pin to trigger the selection process.

// Include the Adafruit NeoPixel library for controlling RGB LEDs.
#include <Adafruit_NeoPixel.h>

// Include a custom library likely for event handling and state management.
// Note: Included twice, potentially a typo, but harmless.
#include <React_Generic.h>
#include <React_Generic.hpp> // Include the header file for the React_Generic library
#include <React_Generic.h>   // Duplicate include of React_Generic library header

// Include the Adafruit Circuit Playground library for board-specific functions.
#include <Adafruit_CircuitPlayground.h>

// Use the namespace defined by the React_Generic library.
using namespace react_generic;

// Pointer to the currently active React_Generic application state instance.
React_Generic *app = nullptr;
// Pointer to hold a newly created React_Generic application state, used for transitions.
React_Generic *new_app = nullptr;

// --- NeoPixel Configuration ---
// const auto NEOPIXEL_PIN = CPLAY_NEOPIXELPIN; // Alternative: Use the built-in NeoPixel pin
// const auto PIXEL_COUNT = 10;                 // Alternative: Use the 10 built-in NeoPixels
const auto NEOPIXEL_PIN = 10;     // Define the Arduino pin connected to the NeoPixel data input.
const auto PIXEL_COUNT = 20;    // Define the total number of NeoPixels on the strip.

// --- Application Configuration ---
const auto NUM_CHOICES = 16;    // Define the maximum number of choices the picker can select from.

// Pointer to the Adafruit NeoPixel object representing the LED strip.
Adafruit_NeoPixel *strip;
// Adafruit_NeoPixel *strip2; // Declared but never used.

/**
 * @brief Represents a single "spin" or selection process.
 *
 * This class handles the logic for choosing a random option and
 * animating the selection on the NeoPixel strip.
 */
class Spin {
  public:
    /**
     * @brief Constructor for the Spin class.
     * @param num_choices The number of possible choices for this spin.
     */
    Spin(int num_choices);

    /**
     * @brief Renders the spinning animation and final choice on the NeoPixel strip.
     * @param strip Pointer to the NeoPixel strip object to control.
     */
    void render_spin(Adafruit_NeoPixel *strip);

    // Member variables
    int choice;      // The randomly selected choice index (0 to num_choices-1).
    int num_choices; // The total number of choices possible for this instance.
};

/**
 * @brief Initializes a new Spin object.
 *
 * Selects a random choice based on the provided number of choices and prints it to Serial.
 * @param num_choices The number of possible choices.
 */
Spin::Spin(int num_choices): num_choices(num_choices) {
  // Generate a random integer between 0 (inclusive) and num_choices (exclusive).
  this->choice = random(num_choices);
  // Log the randomly selected choice index to the Serial monitor.
  Serial.print("Random selection: ");
  Serial.println(this->choice);
}

/**
 * @brief Generates a random, relatively bright color.
 *
 * Creates a 32-bit color value suitable for NeoPixels, ensuring it's not too dim.
 * @param strip Pointer to the NeoPixel strip object (used for the Color() method).
 * @return uint32_t A packed 32-bit RGB color value.
 */
uint32_t random_color(Adafruit_NeoPixel *strip) {
    // Generate random values (0-99) for red, green, and blue components.
    uint8_t r = random(100);
    uint8_t g = random(100);
    uint8_t b = random(100);
    // If the color is very dark (all components < 5), boost red slightly to ensure visibility.
    if ((r < 5) && (g < 5) && (b < 5)) {
      r = 15;
    }
    // Use the NeoPixel library's Color method to pack RGB into a single 32-bit value.
    return strip->Color(r, g, b);
}

/**
 * @brief Executes the visual animation for the spin on the NeoPixel strip.
 *
 * Clears the strip, performs a flashing animation with random pixels and colors,
 * and finally lights up the single pixel corresponding to the chosen option.
 * @param strip Pointer to the NeoPixel strip object to control.
 */
void Spin::render_spin(Adafruit_NeoPixel *strip) {
  // Clear all pixels on the strip.
  strip->clear();
  // Send the updated pixel data to the strip.
  strip->show();
  // Set the brightness for the animation (0-255).
  strip->setBrightness(50);

  // Determine the number of steps in the animation.
  // It's a random number between 5 and 15 (inclusive), but capped by 2 * num_choices.
  auto steps = max(min(15, random(2 * num_choices)), 5);

  // Loop for the calculated number of animation steps.
  for (int i=0; i<steps; i++) {
    // Select a random pixel index on the strip.
    auto random_pixel = random(PIXEL_COUNT);
    // Log the current animation step number.
    Serial.println(i);

    // Clear the strip before lighting the next random pixel.
    strip->clear();
    // Set the randomly chosen pixel to a random color.
    strip->setPixelColor(random_pixel, random_color(strip));
    // Update the physical LEDs.
    strip->show();
    // Pause for 100 milliseconds.
    delay(100);
  }

  // After the animation, clear the strip one last time.
  strip->clear();
  // Calculate the index of the pixel representing the final choice.
  // Assumes choices map to the *last* NUM_CHOICES pixels on the strip.
  int final_pixel_index = this->choice + PIXEL_COUNT - num_choices;
  // Light up the final choice pixel with a random color.
  strip->setPixelColor(final_pixel_index, random_color(strip));
  // Update the physical LEDs to show the result.
  strip->show();
}

/**
 * @brief Sets up the application state for the "spinning" process.
 *
 * Creates a new React_Generic instance configured to perform one spin animation
 * and then transition back to the "waiting" state.
 */
void setup_spin() {
  // Log that the spin state is starting.
  Serial.println("Starting a spin...");
  // Create a new React_Generic instance for the spin state. 'false' likely means non-repeating.
  new_app = new React_Generic(false);

  // Define the action to perform when this state's tick occurs (runs once).
  new_app->onTick([]() {
    // Determine the actual number of choices based on available pixels.
    auto num_choices = min(NUM_CHOICES, PIXEL_COUNT);
    // Warn if the configured number of choices exceeds the pixel count.
    if (num_choices < NUM_CHOICES) {
      Serial.println("Warning, # of choices exceeds # of pixels. Will only use # of pixels.");
    }
    // Create a Spin object to manage the selection and animation.
    Spin spin(num_choices);
    // Run the spin animation on the main NeoPixel strip.
    spin.render_spin(strip);

    // After the spin is complete, transition back to the waiting state.
    setup_waiting();
  });
}

/**
 * @brief Sets up the application state for "waiting" for user input.
 *
 * Creates a new React_Generic instance configured to blink the onboard LED
 * and listen for button presses or pin interrupts to trigger a spin.
 */
void setup_waiting() {
  // Log that the waiting state is being set up.
  Serial.println("setting up waiting...");
  // Create a new React_Generic instance for the waiting state. 'false' might be irrelevant if onRepeat/onInterrupt are used.
  new_app = new React_Generic(false);

  // --- Set up periodic action (LED blink) ---
  // Register a callback to run every 1000 milliseconds.
  new_app->onRepeat(1000, []() {
      // Use a static variable to retain state between calls.
      static bool state = false;
      // Toggle the state and set the Circuit Playground's red LED accordingly.
      digitalWrite(CPLAY_REDLED, state = !state);
  });

  // --- Set up interrupt handlers ---
  Serial.println("  interrupt setup...");
  // Register an interrupt handler for the Left Button (Pin 4 on CPX/CPB).
  // Trigger on RISING edge (button press).
  new_app->onInterrupt(CPLAY_LEFTBUTTON, RISING, []() {
    // When pressed, transition to the spin state.
    setup_spin();
  });
  // Register an interrupt handler for the Right Button (Pin 5 on CPX/CPB).
  // Trigger on RISING edge.
  new_app->onInterrupt(CPLAY_RIGHTBUTTON, RISING, []() {
    // When pressed, just print a message to Serial.
    Serial.println("HELLO RIGHT");
  });

  // --- Set up interrupt handler for external Pin 6 ---
  // Configure Pin 6 as an input with an internal pull-down resistor.
  pinMode(6, INPUT_PULLDOWN);
  // Register an interrupt handler for Pin 6.
  // Trigger on RISING edge (when the pin goes from LOW to HIGH).
  new_app->onInterrupt(6, RISING, []() {
    // Use static variable for simple debounce timing.
    static auto last_action_time = 0;
    // Get the current time in milliseconds.
    auto now = millis();
    // Check if enough time (250ms) has passed since the last trigger.
    if (now - last_action_time > 250) {
      // If debounced, print a message.
      Serial.println("HELLO Morse");
      // Record the time of this action.
      last_action_time = now;
      // Transition to the spin state.
      setup_spin();
    }
  });
}

/**
 * @brief Standard Arduino setup function. Runs once on power-up or reset.
 */
void setup() {
  // Initialize the Circuit Playground library. Set NeoPixel brightness to 50 (0-255).
  CircuitPlayground.begin(50);
  // Seed the random number generator using the analog reading from the sound sensor.
  randomSeed(CircuitPlayground.soundSensor());

  // Initialize the NeoPixel strip object.
  // Params: pixel count, pin number, pixel type flags (GRB color order, 800 KHz speed).
  strip = new Adafruit_NeoPixel(PIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
  // Start the NeoPixel communication.
  strip->begin();
  // Clear any existing colors on the strip.
  strip->clear();
  // Briefly light up the first pixel associated with choices in red as an indicator.
  strip->setPixelColor(PIXEL_COUNT - NUM_CHOICES, strip->Color(255, 0, 0));
  strip->show(); // Update the physical LEDs.

  // Pause for 2 seconds to allow time for setup/viewing the indicator.
  delay(2000);

  // Initialize Serial communication at 115200 baud rate for logging.
  Serial.begin(115200);
  // Print a startup message.
  Serial.println("Setting up spinner...");

  // Set up the initial application state (waiting state). This assigns to new_app.
  setup_waiting();
  // Atomically switch the main application pointer 'app' to the newly created 'new_app'.
  app = new_app;
  // Clear new_app to indicate the transition is complete.
  new_app = nullptr;
}

/**
 * @brief Standard Arduino loop function. Runs repeatedly after setup().
 */
void loop() {
  // --- State Transition Logic ---
  // Check if a new application state has been prepared in 'new_app'.
  if (new_app != nullptr) {
    // Temporarily disable interrupts to safely swap pointers.
    noInterrupts();
    // Delete the old application state object to free memory.
    delete app;
    // Point 'app' to the new state object.
    app = new_app;
    // Clear 'new_app' as the transition is done.
    new_app = nullptr;
    // Re-enable interrupts.
    interrupts();
  }

  // --- Current State Execution ---
  // Call the tick() method of the currently active application state.
  // This processes any pending timers, interrupts, or actions for that state.
  app->tick();
}
