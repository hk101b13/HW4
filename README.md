# HW4_1
I use (x,y) to determine the location of the bbcar, and the origin represents the place we want bbcar back to. Note that the head of car always faces to the direction of the X-axis.

# HW4_2
I use image.get_regression() to get the line in the region(20,0,120,30), and use line.rho() to check the car's direction. If it faces to left, line.rho() is positive, and let the car go right; else if it faces to right, line.rho() is negativ, and let the car go left.

# HW4_3
I use the tag.y_rotation() to check if the car is facing directly and perpendicular to the AprilTag surface. If so, the tag.y_rotation() will be less than 0.1 and uart will write a char "s" to main.cpp. When the main.cpp get the "s", the car will stop rotating and start the ping.
