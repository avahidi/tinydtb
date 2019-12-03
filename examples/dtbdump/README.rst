This utility dumps DTB data to stdout. It turns this::

  stuff = "1234";
  things {
    morestuff = <0, 1>;
  };

. . . into this::

  /stuff = "1234"
  /things/morestuff = <0x00000000,0x00000001>


The idea is to use this for Taco Bell programming...
