#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

    void vmainsetup(void);
    void vmainloop(void);

#ifdef __cplusplus
}
#endif

void setup() {
    vmainsetup();
}

void loop() {
  vmainloop();
}
