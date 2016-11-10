# Smart HRV operation

While trying to build a *smart* HRV controller, we take the following
environmental factors into consideration:

 1. Inside temperature
 2. Inside humidity
 3. Outside temperature
 4. Outside humidity
 5. Home occupancy mode:
    a. Vacant.
    b. Low occupancy/activity (everyone sleeping, one person present, etc.)
    c. Medium occupancy (normal mode.)
    d. High activity (cooking, bathing, etc.)

The controlling factors would be:

 1. Desired interior humidity.
 2. Desired interior temperature.
 3. Desired ventilation level:
   a. Off
   b. "Minimal" manual mode.
   c. Automatic ventilation control.
   d. "Maximum" manual mode.

# General logic

Desired modes of operation "a", "b" and "d" are self-evident:
 a. All relays de-energized.
 b. Low-speed operation on 10min on / 50min off schedule.
 d. High-speed continuous operation.

## Automatic ventilation control

### Output modes

For the sake of simplicity, only 4 modes are introduced:

 A. 10min on / 50min off low speed (energy conservation, low humidity alert mode).
 B. 20min on / 40min off low speed (low activity mode).
 C. Full-time low speed (medium activity mode).
 D. Full-time high speed (high activity mode).

### Operation logic

If *either* of the following conditions is true:
  1. House is vacant
  2. Outdoor absolute humidity is less than indoor absolute humidity and
     indoor relative humidity is less than desirable:

  **Then activate mode A** (energy conservation)

If *either* of the following conditions is true:
  1. Bringing the outside air will change the interior temperature in the desirable
     direction (e.g. outside air is colder than inside, and inside desirable
     temperature is colder than actual):
  2. House is in "high activity" mode.

  **Then activate mode D** (high ventilation)

If *either* of the following conditions is true:
  1. House is in low accuppancy/activity mode.
  2. Outside air is 20 degrees colder than the desirable interior temperature (when
     heating), or 10 degrees hotter than the desirable interior temperature (when cooling).
  3. No updates for interior/exterior sensor data for 2 hours.

  **Then activate mode B** (low ventilation)  

Else **activate mode C** (medium ventilation)


# Humidity control

Since the dehumidistat dry contact is completely separate input on the HRV, this part
of control is completely separate from the generic ventilation control. It would
be possible to amend this to activate high-speed mode on units that do not have a
separate humidistat dry controls, though.

Interior humidity plays a huge role, it is important for humans, pets, plants,
books, wooden floors, and what's not. So its control takes precedence over
vacancy or energy savings:

If interior *ABSOLUTE* humidity is higher than exterior *ABSOLUTE* humidity,
and the exterior temperature is below 15 degrees C, and the interior relative
humidity is above the set value, operate HRV in dehumidistat mode, using PID.

## PID operation

Since we're operating a relay, and a motor, and the whole system has an enormous
reactivity, we turn the motor on and off only once an hour. The *time* the motor
will be on depends on how much we want to change the internal humidity, and how
fast we want to counter the general rise of internal humidity in the house.

### So. PID parameters (initial guess):

Control variable: interior relative humidity
Kp = 2, Ki = 5, Kd = 1
Window size = 3,600 seconds (one hour)

Example control:
https://github.com/br3ttb/Arduino-PID-Library/blob/master/examples/PID_RelayOutput/PID_RelayOutput.ino
