= FingerText

Tab triggered snippet plugin for Notepad++.

== Description

Tab triggered snippet plugin for Notepad++. Support global snippets and snippets for each file extension or file name, hotspot navigation, hotspot text hint(instead of just a $ or # sign) and hotspot default value. You can add comment to you snippets. 

This plugin is still in Alpha (or may be pre-alpha.....) stage so do expect that it is buggy. The development direction is to be at least a working snippet plugin,  and probably a tab triggered simple command plugin, turn the editor into a convenient console and finally add some ruby scripting support.

Again this is alpha and I am still experimenting different methods. So do expect api changes which is not compatible with current version.

== Install

Download the file FingerText - 0.3.1.zip

Unzip the package and copy the files in the plugins folder of the zip file into the plugins folder of Notepad++.

== Usage

To trigger a snippet. Type the snippet tag and hit the [tab] key.

For example, you can open a new file , type in "global"(without quotes) and hit [tab]. 

  global[tab]

The tag will be replaced by "This is a global snippet."

Some snippets are file extension specific. For example, if you open a file called test.cpp, you can type cpp(without quote) and hit [tab]. 
  
  cpp[tab]

The tag will be replaced by "This is a cpp snippet". But if you do the same thing in a file call abc.txt, the snippet will not be triggered.

Some snippets are File name specific. It will be triggered only when the file name matches.

(Notice that it does not depends on which language you choose in notepad++, what matters is the file name!)

Some snippets contain hotspots. For example, open a new file and trigger the snippet "mit" (type "mit"(without quote and hit [tab]). 

  mit[tab]

A sample of mit license. Your cursor is currently at the "year" spot. And you can type in the year (say 2011) and hit [tab] again. The cursor is now on the name spot and you can enter the name.

Some of them contain multiple hotspot with the same name. For example you can trigger the snippet "awesome". Your cursor is on all three spots and anything you type will change all of them simultaneously. 

To see all the available snippets. Go to the the plugin folder of Notepad++, go into the folder FingerText. For example if your notepad++ is installed in C:\Program files\Notepad++, then the snippets will be in C:\Program files\Notepad++\plugins\FingerText

All the snippets are arranged in different folders. The folder "(Snippet)Global" contain snippets that is available everywhere.

To make your own snippet, just use the existing snippets as reference. Remember each file contain one snippet. The folder name indicates the scope and the file name indicates the tag. Just type whatever you like in the snippet file. And REMEMBER to place a [>END<] at the end of each snippet. Also, make sure that you encode your snippet file in ANSI. (The snippet can be triggered in ANSI or UTF-8 file though)

Any thing after the [>END<] string will not be inserted so you can write some comment about your snippet here.

Hotspots are indicated by $[![Whatever name]!]. Hotspots will the same name will be highlighted at the same time.

Again this is alpha so I don't know if I will change these things in the future or not. And there are some sample snippet for C and C++.

Chinese characters are supported in snippets. You can try it by triggering the snippet "chinese" (Still all snippets should be encoded in ANSI, and can be triggered in ANSI or UTF-8)  However, Chinese character tags are not supported. Actually sometimes it works sometimes it doesn't so it is not officially supported. 

== Change log 
(for full change log please refer to the CHANGELOG file in the repository)

0.3.1
Add more details in usage description.
edit some typo in sample template
refactor the tag path part
fixed possible bug of char array manipulation
allow for more hotspots

0.3.0
Implemented Snippet set for specific file name.
The priority is FileName>Ext>Global
performance improvement to path search

0.2.6
Multiple hotspot selection and hotspot hint text replacement works simultaneously

0.2.5
Add Multiple hotspot selection.
hotspot hint text replacement disabled temporarily.

0.2.4
Change in tab navigation behaviour. The hotspot will be replaced by the hint text when using tab navigation

0.2.3
Add support for chinese character tag name. Can be triggered from either ANSI or Utf-8 document (But chinese name mess up the word selection in the editor so it may not trigger correctly under ANSI document. So English tag names are encouraged.)

0.2.2
Add Readme and License
Add basic c++ sample snippets
fixed tag longer than snippet problem
fixed the limited row size problem
Turn on the undo step squash function which is accidentally turned off in previous version
Lots of performance improvement changes

0.2.0
Support utf-8 encoding. (All snippets are assumed to be in ANSI, and will be converted if current page is utf-8)
Fixed long tab problem.
Fixed Spaces or tabspaces before tab problem.
Fixed empty snippet problem.

0.1.2
Emulate tab behaviour when no tag match or selection is rectangle.

0.1.1
Snippet insertion recorded as one single undo step.

0.1.0 
First Alpha verison. Implemented basic tab trigger snippet and hotspot navigation

 
== Tested with

- Notepad++ 3.8.6(Unicode), Windows 7 Ultimate 64bit 


== License

This plugin is built using the NppPluginTemplate provided by Don Ho. The license of the template is included in the source.

The plugin is released under MIT license.

MIT license

Copyright (C) 2011 by Tom Lam

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
