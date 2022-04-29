# Installing this plugin, in 3 easy steps
**Step 1: generate makefiles**

Make sure that sigutils, suscan, SuWidgets and SigDigger have been installed in a system-wide location with `sudo make install`
```
$ cd APTPlugin
$ qmake APTPlugin.pro
```

**Step 2: build**
```
$ make
```

**Step 3: install**
```
$ mkdir -p ~/.suscan/plugins
$ cp libAPTPlugin.so.1.0.0 ~/.suscan/plugins
```
And that's it! :D
