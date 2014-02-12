#Holotouch Architecture

#Statement of purpose
The idea behind this software is that humans interact and manipulate
more easily "tangible" objects in space. I wanted to recreate the sensation
that we have when for example we store things in a box.

To do that I used technologies that already exists alone, but I
put them together to create a full natural interface.
Let me explain some of the methods I used and for what results.

##1. Head Tracking and "*Holography*"
The whole idea of this project originated from [one video](http://www.youtube.com/watch?v=Jd3-eiid-Uw). It show us that using simply a wiimote for following the head of someone we can update
the display of a 3D environment and make it look like it's really in the space of the room.
Here are other examples I found demonstrating this, using also a webcam for tracking the head position:
- A really impressive demonstration using opencv: http://www.youtube.com/watch?v=h9kPI7_vhAU

##2. Touchless control with Leapmotion
As a software who only display things is not enough, I choose to add
the possibility to control it with the hands. For that there is an
amazing sensor called (Leapmotion)[https://www.leapmotion.com]
You can use it's API to get all the informations on the hands
positions above it, informations on the fingers and gestures, it's very
accurate.
My goal is to imitate the kinds of controls we see in movies:
- (Minority Report)[http://www.youtube.com/watch?v=7SFeCgoep1c]
- (Jarvis in Iron Man)[http://www.youtube.com/watch?v=D156TfHpE1Q]

##3. 3D display with OpenGl
Simply to display things in 3D I used the OpenGl Library because it's open source and portable.

##4. Qt5 Gui
I used this library because I'm starting to know it well and it's complete,
each time you want to do a particular thing, qt has a method to help you doing
it much faster. MoreOver it's portable and you can use it with a lgpl agreement.

Using all these technologies in one single project looks huge but I am only using some
parts from each one that I need. I'm trying to create a general interface that is called **Holotouch**, some modular code that can be reused to do different kind of programms using this software. But here my goalis to use it to make a 3D file explorer.

#The 3D file explorer with Holotouch
I chose this aim because it is a good example of how you can manipulate objects in space
and order them, navigate inside, all by using only the hands in mid air.
Here are the common features of a classic file explorer:
- Display the content of a folder:
  - Control the number of items in the view
  - scroll up/down to see everything
  - Display icons according to file type
  - Display miniatures when possible (Pictures, videos, ...)
- Navigate from one folder to another
- Select single or multiple files
- Contextual menu:
  - Copy
  - Cut
  - Delete
  - Paste
- Open a file with according software
- Drag and drop files from/into folders

I want to be able to do all these actions using the hands.
Let's do the specifications of these actions (It can change during developpment if is see one more convenient).

## Gesture Specifications:

We specify gestures from different aspects: Transitions between states, or specific
movement in space.

Naming:
- Right Hand: R
- Left Hand: L
- Palm position in space: P with coordinates x,y,z P(X,Y,Z)
- Hand states
  - Opened: 3 or more fingers visible and sphere radius > of a certain open_treshold
  - Closed: No fingers visible and or radius < closed threshold
  - Pointing: Index (and thumb) visible.
  - Hidden: Hand not in field of view of Leapmotion
- Gestures
  - Swipe: Translation of P in one direction with a certain amplitude A and speed S.
  - Circle: Rotation around a virtual axis with a radius R and speed S

- A -> B : Transition from state A to state B

Our actions:

- Click: R Opened -> Closed -> Opened
- Double Click: Double Click ;)
- Open an item: Double Click on the item
  - If it's a file: Open it with the corresponding application
  - If it's a folder: Change the vew and display the content of the folder

- Set Selection Mode:
  - Copy: L visible Open
  - Cut: L visible Closed
- Select single item:L Hidden,  R One click on the item,
  visual highlight like Growing or change of color. When selecting another item,
  deselect precedent.

- Multiple Selection: L Visible, R Select item,
  when selecting another item, previous one stays selected.
- Single Grab: R Opened -> Closed near an item. Then stays closed and moves away,
  the item follows RP
- Multiple Grab: do Multiple Select, then put R away from items (still closed) then Opened.
  All items goes to RP
- Drop: When grabbing, re Open Hand.
  - If near a folder item: perform past operation into the folder
  - If nowhere: cancel operation, every item goes back in place


# Code Architecture

The code is based on tracking objects (head and hands) it means that we need to have
this tracking part running in parallel to the display in order to make it reactive.
We also use the gui library Qt, wich allows a lot of things like events managment,
but the underlying threads are not really visible. So we need to make the better use
of qt's events and signal/slots to make a real multithread and safe program.

There are main modules that handles every aspect of the program:
- Head tracking (using opencv)
- Hand tracking (using leapmotion api)
- 3D display (Using Qt implementation of OpenGl)
- General 2D Gui/core part using Qt5.

in the following I will explain how each module work and how they synchronize and
exchange informations between each other.

##1. Head Tracking
