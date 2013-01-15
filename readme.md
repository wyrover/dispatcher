# Dispatcher

[Dispatcher](https://github.com/rbewley4/dispatcher) is a task queue and worker thread class
for C++.

## License

####Dispatcher
####Version 3.0
####Copyright (C) 2012-2013 Russell Bewley

Dispatcher is free software released under the [MIT License](http://www.opensource.org/licenses/mit-license.php).

See license.txt for more information.

## Dependencies

Dispatcher depends on the following libraries:

- [boost](http://boost.org) - Dispatcher depends on the following boost packages: Bind, Date_Time, Function, Smart Pointers, Thread, and Utility.
- [VLT](http://github.com/rbewley4/vlt) - for tests

## Change Log

* Version 3.0
  * fixes worker thread deadlock 
  * refactored classes
  * WARNING: not backwards compatible with previous versions.
* Version 2.0 
  * added support for recurring events
  * workaround for worker thread deadlock issue
* Version 1.0
  * initial release