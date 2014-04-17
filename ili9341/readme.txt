Dependencies
============

Any application that links with this library must also
link with the dspic_hal library found here:

https://github.com/fernando-rodriguez/fat32lib/tree/master/dspic_hal

or a suitable spi driver must be developed for the target
device.

You must also make sure that spi.h (included with the above
mentioned library) is included in the include search path 
before compiling.