del *.bin /s
del *.lst /s
del *.txt /s
del *.uvopt /s
del *.uvgui.* /s
del *.uvguix.* /s
rd obj /s /q
rd RTE /s /q
rd list /s /q
rd Lst /s /q
rd matismart /s /q
rd DebugConfig /s /q
del *.bak /s
del *.ddk /s
del *.edk /s
del *.lst /s
del *.lnp /s
del *.mpf /s
del *.mpj /s
del *.obj /s
del *.omf /s
::del *.opt /s  ::不允许删除JLINK的设置
del *.plg /s
del *.rpt /s
del *.tmp /s
del *.__i /s
del *.crf /s
del *.o /s
del *.d /s
del *.axf /s
del *.tra /s
del *.dep /s
del JLinkLog.txt /s

del *.iex /s
del *.htm /s
del *.sct /s
del *.map /s
del *.dep /s
del *.ewt /s
del *.log /s
del *.ewd /s
del Backup* /s
rd settings /s /q
exit
