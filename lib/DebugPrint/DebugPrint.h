#ifndef __DEBUG_PRINT_H__
#define __DEBUG_PRINT_H__

extern bool DEBUG_MODE;

#define DebugPrintln(a) (DEBUG_MODE ? Serial.println(a) : false)
#define DebugPrint(a) (DEBUG_MODE ? Serial.print(a) : false)

#endif /* __DEBUG_PRINT_H__ */
