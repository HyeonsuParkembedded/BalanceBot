#ifndef UNITY_CONFIG_H
#define UNITY_CONFIG_H

// Unity configuration for native testing environment
#define UNITY_INCLUDE_64
#define UNITY_INCLUDE_FLOAT
#define UNITY_SUPPORT_64
#define UNITY_FLOAT_PRECISION 0.00001f
#define UNITY_DOUBLE_PRECISION 0.0000001

// Output configuration
#define UNITY_OUTPUT_COLOR

// UNITY_INCLUDE_DOUBLE is already defined in platformio.ini build_flags

#endif // UNITY_CONFIG_H