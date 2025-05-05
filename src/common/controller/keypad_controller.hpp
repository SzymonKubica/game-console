#include "controller.hpp"

#define LEFT_BUTTON_PIN 9
#define DOWN_BUTTON_PIN 15
#define UP_BUTTON_PIN 8
#define RIGHT_BUTTON_PIN 12

class KeypadController : public Controller
{
      public:
        /**
         * For a given controllers, this function will inspect its state to
         * determine if an input is being entered. Note that for physical
         * controllers, this function only tests for the state of the controller
         * right now (it doesn't poll for a period of time). Becuase of this,
         * this function should be called in a loop if we want the system to
         * wait for the user to provide input.
         *
         * If an input is registered, it will be written into the `Direction
         * *input` parameter and `true` will be returned.
         *
         * If no input is registered, this function returns false and the
         * direction pointer remains unchanged.
         */
        bool poll_for_input(Direction *input) override;

        /**
         * Setup function used for e.g. initializing pins of the controller.
         * This is to be called only once inside of the `setup` Arduino
         * function.
         */
         void setup() override;

        KeypadController(int (*digital_read_)(unsigned char),
                           void (*pin_mode_)(unsigned char, unsigned char))
            : digital_read(digital_read_), pin_mode(pin_mode_)
        {
        }

      private:
        /**
         * The digital read function that is provided by the Arduino core layer.
         * Allows for testing the state of the buttons on the keypad.
         *
         * This is to be passed in when constructing the keypad controller.
         * The reason is that we cannot import the Arduino specific functions
         * inside of the C++ sources.
         *
         */
        int (*digital_read)(unsigned char);

        /**
         * The arduino function used for assigning pin modes. Used for
         * initializing the pins used by the joystick controller
         *
         * This is to be passed in when constructing the joystick controller.
         * The reason is that we cannot import the Arduino specific functions
         * inside of the C++ sources.
         *
         */
        void (*pin_mode)(unsigned char, unsigned char);
};
