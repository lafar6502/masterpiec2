

#ifndef rotary_h
#define rotary_h



// Enable this to emit codes twice per step.
#define HALF_STEP



// Values returned by 'process'
// No complete step yet.
#define DIR_NONE 0x0
// Clockwise step.
#define DIR_CW 0x10
// Anti-clockwise step.
#define DIR_CCW 0x20

class Rotary
{
  public:
    Rotary();
    // Process pin(s)
    unsigned char process(bool v1, bool v2);
  private:
    unsigned char state;
};

#endif