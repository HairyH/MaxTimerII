# MaxTimerII
MaxTimerII Paltalk Timer Readme

MaxTimerII.exe is a timer for the new (Qt based) Paltalk chat program. It is timing how long chatter is talking. At its basic level, it is a simple debugger, starting and controlling the Paltalk program. It getting debug strings from the Paltalk program, indicating when a chatter is talking and releasing the microphone (mic). For this reason, it should be run by itself alone on a Windows computer.

Installation:
Since the nature of the MaxTimerII.exe (controlling another program) the computers virus protection, like Windows Defender, may detect it as a dangerous program and remove it automatically from the system. It is not a virus or a dangerous program. You can verify it as the source code available at https://github.com/HairyH/MaxTimerII.git . You can check, or if you do not trust the binary program, compile it yourself, using VS2022 development environment.
So, before you download, or extract the exe file from the zip file, you should temporary disable you virus protection monitoring. Extract the MaxTimerII.exe from the zip file and put it in a folder or onto your Desktop. In your virus protection system make an exception to the MaxTimerII.exe and switch the monitoring back on.

Usage:
Usage is simple; make sure that Paltalk is not running! Then double click on the MaxTimerII.exe and it will open a small console app window and start Paltalk for you. When Paltalk is running, login and open the room you like to time. When the room is open and running, go to the MaxTimerII console window and enter 1 at the prompt. If the Paltalk room is not open, or you encounter some other fault, enter 2 at the prompt. That will shut the console and Paltalk as well. 
If it’s running, it will write some information about the Paltalk program in the console window. The last one is “Debugger Attached.” Once this happens, the timer is running and will log events in the console window.
Do not close the console  window, unless you want to shut Paltalk down.  
