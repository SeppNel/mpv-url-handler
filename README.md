URL Handler for mpv player.  
# Usage
Extract to folder of choice and run exe file. It should create the registry keys necessary.  
In config.yml change mpvPath to you mpv player executable path.  
After that any url starting with mpv:// should be opened with this, and if everything is okay it should play with mpv.  

Example url: 
```
mpv://http://myserver.com/video.mkv
```

Valid protocols are http and https, although is easy to add more in the code.

# Compiling
Download [m4x1m1l14n Registry library](https://github.com/m4x1m1l14n/Registry/tree/master) and place it in the same folder.  
Visual Studio solution provided.
