/******************************************************************************
 *
 * This file contains the default JSON file input.
 *
 *****************************************************************************/

#define STRINGIFY_JSON(...) #__VA_ARGS__

const char onlp_platform_sim_default_json[] = STRINGIFY_JSON(
{
	"hdr":	{
		"id":	"chassis-1",
		"description":	null,
		"poid":	null,
		"coids":	["thermal-1", "thermal-2", "thermal-3", "thermal-4", "thermal-5", "led-1", "led-2", "led-3", "fan-5", "fan-6", "led-4", "led-5", "psu-1", "psu-2", "fan-1", "fan-2", "fan-3", "fan-4"],
		"status":	["PRESENT", "OPERATIONAL"]
	},
	"coids":	{
		"thermal-1":	{
			"hdr":	{
				"id":	"thermal-1",
				"description":	"CPU Core",
				"poid":	null,
				"coids":	[],
				"status":	["PRESENT"]
			},
			"caps":	["GET_TEMPERATURE", "GET_WARNING_THRESHOLD", "GET_ERROR_THRESHOLD", "GET_SHUTDOWN_THRESHOLD"],
			"mcelsius":	18000,
			"warning-threshold":	45000,
			"error-threshold":	55000,
			"shutdown-threshold":	60000
		},
		"thermal-2":	{
			"hdr":	{
				"id":	"thermal-2",
				"description":	"Chassis Thermal Sensor 1",
				"poid":	null,
				"coids":	[],
				"status":	["PRESENT"]
			},
			"caps":	["GET_TEMPERATURE", "GET_WARNING_THRESHOLD", "GET_ERROR_THRESHOLD", "GET_SHUTDOWN_THRESHOLD"],
			"mcelsius":	24500,
			"warning-threshold":	45000,
			"error-threshold":	55000,
			"shutdown-threshold":	60000
		},
		"thermal-3":	{
			"hdr":	{
				"id":	"thermal-3",
				"description":	"Chassis Thermal Sensor 2",
				"poid":	null,
				"coids":	[],
				"status":	["PRESENT"]
			},
			"caps":	["GET_TEMPERATURE", "GET_WARNING_THRESHOLD", "GET_ERROR_THRESHOLD", "GET_SHUTDOWN_THRESHOLD"],
			"mcelsius":	21500,
			"warning-threshold":	45000,
			"error-threshold":	55000,
			"shutdown-threshold":	60000
		},
		"thermal-4":	{
			"hdr":	{
				"id":	"thermal-4",
				"description":	"Chassis Thermal Sensor 3",
				"poid":	null,
				"coids":	[],
				"status":	["PRESENT"]
			},
			"caps":	["GET_TEMPERATURE", "GET_WARNING_THRESHOLD", "GET_ERROR_THRESHOLD", "GET_SHUTDOWN_THRESHOLD"],
			"mcelsius":	22000,
			"warning-threshold":	45000,
			"error-threshold":	55000,
			"shutdown-threshold":	60000
		},
		"thermal-5":	{
			"hdr":	{
				"id":	"thermal-5",
				"description":	"Chassis Thermal Sensor 4",
				"poid":	null,
				"coids":	[],
				"status":	["PRESENT"]
			},
			"caps":	["GET_TEMPERATURE", "GET_WARNING_THRESHOLD", "GET_ERROR_THRESHOLD", "GET_SHUTDOWN_THRESHOLD"],
			"mcelsius":	20000,
			"warning-threshold":	45000,
			"error-threshold":	55000,
			"shutdown-threshold":	60000
		},
		"led-1":	{
			"hdr":	{
				"id":	"led-1",
				"description":	"Chassis LED 1 (DIAG LED)",
				"poid":	null,
				"coids":	[],
				"status":	["PRESENT"]
			},
			"caps":	["OFF", "RED", "ORANGE", "GREEN"],
			"mode":	"GREEN"
		},
		"led-2":	{
			"hdr":	{
				"id":	"led-2",
				"description":	"Chassis LED 2 (LOC LED)",
				"poid":	null,
				"coids":	[],
				"status":	["PRESENT"]
			},
			"caps":	["OFF", "BLUE"],
			"mode":	"OFF"
		},
		"led-3":	{
			"hdr":	{
				"id":	"led-3",
				"description":	"Chassis LED 3 (FAN LED)",
				"poid":	null,
				"coids":	[],
				"status":	["PRESENT"]
			},
			"caps":	["AUTO"],
			"mode":	"AUTO"
		},
		"fan-5":	{
			"hdr":	{
				"id":	"fan-5",
				"description":	"Chassis Fan 5",
				"poid":	null,
				"coids":	[],
				"status":	["PRESENT"]
			},
			"caps":	["GET_DIR", "SET_PERCENTAGE", "GET_RPM", "GET_PERCENTAGE"],
			"dir":	"F2B",
			"rpm":	7600,
			"percentage":	42
		},
		"fan-6":	{
			"hdr":	{
				"id":	"fan-6",
				"description":	"Chassis Fan 6",
				"poid":	null,
				"coids":	[],
				"status":	["PRESENT"]
			},
			"caps":	["GET_DIR", "SET_PERCENTAGE", "GET_RPM", "GET_PERCENTAGE"],
			"dir":	"F2B",
			"rpm":	7400,
			"percentage":	41
		},
		"led-4":	{
			"hdr":	{
				"id":	"led-4",
				"description":	"Chassis LED 4 (PSU1 LED)",
				"poid":	null,
				"coids":	[],
				"status":	["PRESENT"]
			},
			"caps":	["AUTO"],
			"mode":	"AUTO"
		},
		"led-5":	{
			"hdr":	{
				"id":	"led-5",
				"description":	"Chassis LED 4 (PSU2 LED)",
				"poid":	null,
				"coids":	[],
				"status":	["PRESENT"]
			},
			"caps":	["AUTO"],
			"mode":	"AUTO"
		},
		"psu-1":	{
			"hdr":	{
				"id":	"psu-1",
				"description":	"PSU-1",
				"poid":	null,
				"coids":	["fan-7", "thermal-6"],
				"status":	["PRESENT", "UNPLUGGED"]
			},
			"caps":	["GET_TYPE", "GET_VOUT", "GET_IOUT", "GET_POUT"],
			"type":	"AC",
			"mvout":	11906,
			"miout":	8968,
			"mpout":	106000,
			"coids":	{
				"fan-7":	{
					"hdr":	{
						"id":	"fan-7",
						"description":	"Chassis PSU-1 Fan 1",
						"poid":	null,
						"coids":	[],
						"status":	["PRESENT"]
					},
					"caps":	["GET_DIR", "SET_PERCENTAGE", "GET_RPM", "GET_PERCENTAGE"],
					"dir":	"F2B",
					"rpm":	4800,
					"percentage":	18
				},
				"thermal-6":	{
					"hdr":	{
						"id":	"thermal-6",
						"description":	"PSU-1 Thermal Sensor 1",
						"poid":	"psu-1",
						"coids":	[],
						"status":	["PRESENT"]
					},
					"caps":	["GET_TEMPERATURE", "GET_WARNING_THRESHOLD", "GET_ERROR_THRESHOLD", "GET_SHUTDOWN_THRESHOLD"],
					"mcelsius":	27000,
					"warning-threshold":	45000,
					"error-threshold":	55000,
					"shutdown-threshold":	60000
				}
			}
		},
		"psu-2":	{
			"hdr":	{
				"id":	"psu-2",
				"description":	"PSU-2",
				"poid":	null,
				"coids":	[],
				"status":	["PRESENT", "UNPLUGGED"]
			},
			"caps":	[]
		},
		"fan-1":	{
			"hdr":	{
				"id":	"fan-1",
				"description":	"Chassis Fan 1",
				"poid":	null,
				"coids":	[],
				"status":	["PRESENT"]
			},
			"caps":	["GET_DIR", "SET_PERCENTAGE", "GET_RPM", "GET_PERCENTAGE"],
			"dir":	"F2B",
			"rpm":	7600,
			"percentage":	42
		},
		"fan-2":	{
			"hdr":	{
				"id":	"fan-2",
				"description":	"Chassis Fan 2",
				"poid":	null,
				"coids":	[],
				"status":	["PRESENT"]
			},
			"caps":	["GET_DIR", "SET_PERCENTAGE", "GET_RPM", "GET_PERCENTAGE"],
			"dir":	"F2B",
			"rpm":	7600,
			"percentage":	42
		},
		"fan-3":	{
			"hdr":	{
				"id":	"fan-3",
				"description":	"Chassis Fan 3",
				"poid":	null,
				"coids":	[],
				"status":	["PRESENT"]
			},
			"caps":	["GET_DIR", "SET_PERCENTAGE", "GET_RPM", "GET_PERCENTAGE"],
			"dir":	"F2B",
			"rpm":	7600,
			"percentage":	42
		},
		"fan-4":	{
			"hdr":	{
				"id":	"fan-4",
				"description":	"Chassis Fan 4",
				"poid":	null,
				"coids":	[],
				"status":	["PRESENT"]
			},
			"caps":	["GET_DIR", "SET_PERCENTAGE", "GET_RPM", "GET_PERCENTAGE"],
			"dir":	"B2F",
			"rpm":	7600,
			"percentage":	42
		}
	}
}
                                                             );
