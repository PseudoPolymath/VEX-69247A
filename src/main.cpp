#include "main.h"
#include "pros/misc.h"
#include "pros/rtos.hpp"
#include "pros/screen.h"

#include "shooter.hpp"
#include "roller.hpp"
#include "succ.hpp"

#include "replay.hpp"
#include "drivetrain.hpp"
#include <chrono>

using namespace std::chrono_literals;

// MOTOR DEFINITIONS
pros::Controller drive_con(pros::E_CONTROLLER_MASTER);

bool end_game_available;
std::chrono::milliseconds op_adjust;

void set_tank(int l, int r) {
	left_back = l;
	right_back = r;
	left_front = l;
	right_front = r;
}

void initialize() {
	pros::Task task{[=] {
		pros::delay(100*1000);
		end_game_available = true;
		std::cout << "End Game avalible" << std::endl;
	}};
}

void autonomous() {
	init_drivetrain();
	roller.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);

	// Replay
	VirtualController virtual_con(&drive_con, true);
	std::chrono::high_resolution_clock clock;

	while(true) {
		// Get recorded frame
		virtual_con.read_from_file();

		auto t1 = clock.now(); // Start record
		drive_auton(&virtual_con);
	
		roller_auton(&virtual_con);

		// SUCC Control
		succ_auton(&virtual_con);

		shoot_auton(&virtual_con);

		if (drive_con.get_digital(pros::E_CONTROLLER_DIGITAL_DOWN) && end_game_available) {
			std::cout << "End game used" << std::endl;
			end_game_available = false;
		}

		// Record time for replay adjustment
		auto t2 = clock.now();
		std::chrono::milliseconds ms_adjust = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
		//auton_adjust = ms_adjust;
		pros::delay(20 + (op_adjust - ms_adjust).count());
	}	
}

void opcontrol()
{
	init_drivetrain();
	roller.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);

	// Replay
	VirtualController virtual_con(&drive_con, false);
	std::chrono::high_resolution_clock clock;

	while(true) {
		auto t1 = clock.now(); // Start record
		drive_op(&drive_con);
	
		roller_op(&drive_con);

		// SUCC Control
		succ_op(&drive_con);

		shoot_op(&drive_con);


		if (drive_con.get_digital(pros::E_CONTROLLER_DIGITAL_DOWN) && end_game_available) {
			std::cout << "End game used" << std::endl;
			end_game_available = false;
		}

		// Replay code
		virtual_con.record_frame();
		virtual_con.write_to_file();

		// Record time for replay adjustment
		std::chrono::milliseconds ms_adjust = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
		std::cout << "Op control took " << ms_adjust.count() << " ms" << std::endl;
		op_adjust = ms_adjust;
		pros::delay(20);
	}	
}
