// Need to add ei_examples.zip library to Arduino IDE
#include "ei_camera_result.h"

#include <micro_ros_arduino.h>

#include <stdio.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <ei_interfaces/msg/ei_result.h>
#include <ei_interfaces/msg/ei_classification.h>

rcl_publisher_t publisher;
ei_interfaces__msg__EIResult msg;
rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_timer_t timer;

#define LED_PIN 13

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}


void error_loop(){
  while(1){
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(100);
  }
}

void timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{  
  RCLC_UNUSED(last_call_time);
    if (timer != NULL) {
    ei_main(msg);
    RCSOFTCHECK(rcl_publish(&publisher, &msg, NULL));
  }
}

void setup() {
  ei_setup();
  
  set_microros_transports();
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  
  
  delay(1000);

  allocator = rcl_get_default_allocator();

  //create init_options
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // create node
  RCCHECK(rclc_node_init_default(&node, "micro_ros_arduino_node", "", &support));

  // create publisher
  RCCHECK(rclc_publisher_init_default(
    &publisher,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(ei_interfaces, msg, EIResult),
    "/micro_ros_arduino_node_publisher"));

  // create timer,
  const unsigned int timer_timeout = 200;
  RCCHECK(rclc_timer_init_default(
    &timer,
    &support,
    RCL_MS_TO_NS(timer_timeout),
    timer_callback));

  // create executor
  RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
  RCCHECK(rclc_executor_add_timer(&executor, &timer));

  msg.result.capacity = EI_CLASSIFIER_LABEL_COUNT;
  msg.result.data = (ei_interfaces__msg__EIClassification*) malloc(msg.result.capacity * sizeof(ei_interfaces__msg__EIClassification));
  msg.result.size = 0;

    for (int32_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
      msg.result.data[ix].label.capacity = 20;
      msg.result.data[ix].label.data = (char*) malloc(msg.result.data[ix].label.capacity * sizeof(char));
      msg.result.data[ix].label.size = 0;
//        
//      strcpy(msg.result.data[ix].label.data, "test");
//      //msg.result.data[ix].label.size = strlen(msg.result.data[ix].label.data);
//  
//      msg.result.data[ix].value = 0.5;
//    
      msg.result.size++;
    }
}

void loop() {
  delay(50);
  RCSOFTCHECK(rclc_executor_spin(&executor));
}