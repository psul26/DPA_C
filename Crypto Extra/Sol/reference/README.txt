This program is part of the DPA Contest. For more informations about the 
contest, take a look at the following URL: http://www.dpacontest.org

Side-Channel traces to use:
Some side channel traces have been stored on the PostgreSQL server dpa.enst.fr
at the Telecom-Paristech University. They are organised by campaigns stored
in distinct database tables. Each campaign/table contains side-channel traces
measured by our acquisition platform on the same crypto-processor, the same
day, using the same key. Each trace is associated to some informations, like 
the clear message, the key and the cryptogram. 
You can access to these traces using a PostgreSQL client on your computer with
username "guest" and password "guest", or directly with your program.
The list of the tables that can be used for this contest with their
description is maintained up to date on the dpa contest web site: 
http://www.dpacontest.org/tables.php

Provided implementation:
We provide a reference implementation written in python, which implements a
differential power analysis based on the partitioning algorithm published by
Paul Kocher. The application needs an internet connection in order to access
to the side-channel traces. The code can be reused by participants to submit
new algorithms, and participate to the contest.

Reference implementation performance:
The reference implementation needs 2766 power consumption traces to break the
key of the default campaign "secmatv1_2006_04_0809" after having been
stabilized for 100 iterations on the good key.

Needed CPU and memory:

The reference implementation takes 4 hours and a half (resp. about one night)
on an Intel Xeon at 3.00 GHz (resp. Intel Core2 Duo at 1.66 GHz) using only
one core to break the key.
The application will use about 500 MB for itself (depending of the traces
length). Make sure the computer used to run the application has enough memory.
If your computer begins to swap, the computation might never end.

In order to run the provided application, you need to install the following
softwares:
- The PostgreSQL client with his shared libraries (the so called libpq)
- Python
- pyPgSQL
- egenix-mx-base - Allows the use of the DateTime module

The application has been fully tested on the following configurations:
. Windows Vista with:
	- PostgreSQL 8.3
	- Python 2.4.4
	- pyPgSQL 2.5.1
	- egenix-mx-base 3.1.1
. Linux Debian with:
	- PostgreSQL 8.1 (package lipq-dev)
	- Python 2.4.4
	- pyPgSQL 2.5.1
	- egenix-mx-base 3.1.1

Windows specific configuration:
On windows, the bin directory of the PostgreSQL installation directory has
to be added to the windows path environment variable, in order to make it's
DLL accessible from python.

To launch the application just execute the main.py script:
$ python main.py

