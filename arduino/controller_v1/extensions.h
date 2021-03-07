// just use it to display or transfer or to string conversion but not to use for
// math operation
//#define round1dec(a) floorf(a*10)/10                       // round float to 1
// decimal points
float round1dec(float a) {
  return floorf(a * 10) / 10; // round float to 1 decimal points
}

using function = void(void);
using function_pointers = void (*[])(void);
