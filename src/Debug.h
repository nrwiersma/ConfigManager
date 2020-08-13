#ifndef CONFIGMANAGER_DEBUG_H
#define CONFIGMANAGER_DEBUG_H

extern bool DEBUG_MODE;

#define DebugPrint(a) (DEBUG_MODE ? Serial.print(a) : false)
#define DebugPrintln(a) (DEBUG_MODE ? Serial.println(a) : false)

#endif  // CONFIGMANAGER_DEBUG_H
