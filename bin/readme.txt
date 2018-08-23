= About =

Notepad++ Linter plugin allows realtime code check against any checkstyle-compatible linter: jshint, eslint, jscs, phpcs, csslint and many others.

Copyright (c) 2016 Vladimir Soshkin <deadem@gmail.com> MIT License

= Installation =

- Copy linter.dll to your Notepad++ plugins directory, restart Notepad++
- Plugins -> Linter -> Edit config, add configuration file (see example)
- Restart Notepad++
- Have fun

= Config example =

<?xml version="1.0" encoding="utf-8" ?>
<NotepadPlus>
  <linter extension=".js" command="C:\Users\deadem\AppData\Roaming\npm\eslint.cmd --format checkstyle"/>
  <linter extension=".js" command="C:\Users\deadem\AppData\Roaming\npm\jscs.cmd --reporter=checkstyle"/>
  <linter extension=".js" command="C:\Users\deadem\AppData\Roaming\npm\jshint.cmd --reporter=checkstyle"/>
  <linter extension=".css" command="C:\Users\deadem\AppData\Roaming\npm\csslint.cmd --format=checkstyle-xml"/>
  <linter extension=".js" command="&quot;C:\Path with spaces\somelint.cmd&quot; --format checkstyle"/>
</NotepadPlus>

== Optional config tags ==

=== style ===
"color" attribute is a "RGB" hex color.
"alpha" value can range from 0 (completely transparent) to 255 (no transparency).

<?xml version="1.0" encoding="utf-8" ?>
<NotepadPlus>
  <style color="0000FF" alpha="100" />

  <linter ... />
  <linter ... />
</NotepadPlus>
