grader中是之前错误时的截图

错在STW和STB中对memory的写入操作放在了get_internal_signal()中

应该放在latch_datapath_values()中