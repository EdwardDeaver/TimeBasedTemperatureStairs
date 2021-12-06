
use std::error::Error;
use std::thread;
use std::time::Duration;

use rppal::pwm::{Channel, Polarity, Pwm};
use chrono::{Datelike, Timelike, Utc, Local};



fn main() -> Result<(), Box<dyn Error>> {
    // Enable PWM channel 0 (BCM GPIO 18, physical pin 12) at 2 Hz with a 25% duty cycle.
    let pwm = Pwm::with_frequency(Channel::Pwm0, 8000.0, 1.0, Polarity::Normal, true)?;

    // Sleep for 2 seconds while the LED blinks.
    thread::sleep(Duration::from_secs(10));

    // Reconfigure the PWM channel for an 8 Hz frequency, 50% duty cycle.
    pwm.set_frequency(1.0, 0.0)?; 
let now = Local::now();

    let hour = now.hour();
 let hourString = hour.to_string();
let hourInt: i32 = hourString.parse().unwrap();

	println!("{}", hourInt);
	println!("{}", 6200 / hourInt); 
    println!(
        "The current UTC time is {:02}:{:02}:{:02}",
        hour,
        now.minute(),
        now.second()    );

    println!("Hello World!");
    thread::sleep(Duration::from_secs(30));

    Ok(())

    // When the pwm variable goes out of scope, the PWM channel is automatically disabled.
    // You can manually disable the channel by calling the Pwm::disable() method.
}
