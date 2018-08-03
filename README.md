# Notepad++ Linter

 - Copy bin/linter.dll to your Notepad++ plugins directory
 - Plugins -> Linter -> Edit config
 - Restart Notepad++

## Config example

```xml
<?xml version="1.0" encoding="utf-8" ?>
<NotepadPlus>
  <linter extension=".js" command="C:\Users\deadem\AppData\Roaming\npm\jscs.cmd --reporter=checkstyle"/>
  <linter extension=".js" command="C:\Users\deadem\AppData\Roaming\npm\jshint.cmd --reporter=checkstyle"/>
  <linter extension=".js" command="C:\Users\deadem\AppData\Roaming\npm\eslint.cmd --format checkstyle"/>
  <linter extension=".js" command="&quot;C:\Path with spaces\somelint.cmd&quot; --format checkstyle"/>
</NotepadPlus>
```
