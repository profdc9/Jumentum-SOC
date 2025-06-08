REM <oldstate>=SETIDLE(<idlestate>)
REM Sets whether or not the idle task should be run during program execution.
REM <idlestate> is zero for normal idle task running, and nonzero to suspend idle
REM tasks.  The idle task includes network services, so network services will not
REM resume until "x=SETIDLE(0)" is used.  The old idle state is returned in 
REM <oldstate>.   This function is useful to make sure the maximum performance is
REM available for critical tasks.  However, if the idle state is accidentally left
REM disabled, network access will not be available, and the controller may be unreachable
REM by the network.
REM 
c=SETIDLE(1)
PRINT "Network should not be accessible"
FOR dly=1 TO 200000
NEXT dly
c=SETIDLE(c)
PRINT "Network should be accessible"
FOR dly=1 TO 200000
NEXT dly
