/* Layout padding. The Vita ELF converter needs a little free room after the code segment;
   fixpad.sh resizes this array if the converter reports
   "Cannot allocate N bytes for SCE data at end of segment 0". */
const unsigned char leftseat_pad[16] __attribute__((used));
