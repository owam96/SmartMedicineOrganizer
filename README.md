# Smart Medicine Organizer

This repository contatins only the embedded part of the project and not the PC application part, since it is the part I worked on. The original abstract of the project is provided to give the motivation and the goal of the project

### Abstract

For elderly people taking multiple medications every day, it can be difficult to remember when or what medicines the patient should take. It is common to take medicine at the wrong time. For example, age-related memory decline or confusion is common among elderly patients, who are more likely to be unable to take their medication on time or comply with their doctors’ prescriptions.

This project is a proof of concept of a smart medicine organizer which patients can benefit from in the duration of their treatment. By using this device, patients will be able to consume their medicines on time based on their prescriptions. The device consists of numbered compartments, each containing only one type of pill for easy refilling. Each compartment has an LED light that lights when it is time to take the medicine. A color screen displays a welcome screen, relevant information at each time of medicine-taking, and also shows the current inventroy.

A PC application stores the patient’s information and the doctor can use it to register the time and dose of each pill, and through it he can track whether the patient is taking his pills regularly. The application and the device communicate over the internet. Patients can use this smart organizer to keep track of how many pills are left and when
they should see their doctor again.

## Harware Devices

- STM32F429 Nucleo-144 microcontroller
- ILI9341 TFT color screen
- LEDs

## Technologies
- STM32F4 official drivers
- Unity C test harness
- FreeRTOS
- TCP/IP communiaction
