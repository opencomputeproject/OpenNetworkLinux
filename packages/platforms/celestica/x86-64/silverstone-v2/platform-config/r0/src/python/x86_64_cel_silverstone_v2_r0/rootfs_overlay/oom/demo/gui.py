#
# gui.py - a graphical demo of OOM
# Copyright 2017 Finisar Inc.
# Author: Don Bollinger don@thebollingers.org
#
from oom import *
from oom.oomlib import type_to_str
from oom.decode import get_hexstr
from DOMgui import draw_DOM
import sys

from Tkinter import *


def portstr(port):
    formatstr = '%-11s %-13s %-16s %-16s %-16s'
    modtype = type_to_str(port.port_type)
    if modtype == 'UNKNOWN':
        return "%-11s No Module" % port.port_name
    return formatstr % (port.port_name,
                        modtype,
                        oom_get_keyvalue(port, 'VENDOR_PN'),
                        oom_get_keyvalue(port, 'VENDOR_SN'),
                        oom_get_keyvalue(port, 'VENDOR_NAME'))


killlist = []


def kill_widgets():
    for item in killlist:
        item.destroy()


# helper function to make the code easier to read
def pi(p, text):
    p.insert(END, text)


# prints out the sample code screen
def printcode(newroot, pb, port, portnum):
    newroot.title('Code Sample: -- %s' % portstr(port))
    pb.config(font=("Courier", 12, ""))
    pi(pb, 'A complete python script to report DOM status on this port:')
    pi(pb, '')
    pi(pb, 'from oom import *')
    pb.itemconfig(END, foreground="blue")
    pi(pb, 'list = oom_get_portlist()')
    pb.itemconfig(END, foreground="blue")
    pi(pb, 'print oom_get_memory(list[%s], "DOM")' % portnum)
    pb.itemconfig(END, foreground="blue")
    pi(pb, '')
    pi(pb, 'Output:')
    pi(pb, '')

    #
    # kludge, assumes the sample code (above) matches
    # the actual code (below)
    # note also, lots of prettifying the ugly output
    # that python prints by default
    first = True
    chr0 = '{'
    dom = oom_get_memory(port, "DOM")
    for key, value in sorted(dom.items()):
        if not first:
            pb.insert(END, printstr)
            pb.itemconfig(END, foreground="red")
        first = False
        printstr = "%s'%s': %.4f," % (chr0, key, value)
        chr0 = ' '
    s = printstr[0:-1] + '}'
    pb.insert(END, s)
    pb.itemconfig(END, foreground="red")


class OOMdemo:
    def __init__(self, master):
        # load the json shim if a URL has been provided
        parms = sys.argv
        if len(parms) > 1:
            if parms[1] == '-url':
                oomlib.setshim("oomjsonshim", parms[2])

        # put up the main inventory screen
        self.portlist = oom_get_portlist()
        master.title("OOM Demo: Switch Module Inventory")
        self.frame = Frame(master)
        self.frame.pack()

        # header at the top of the screen
        self.header = Listbox(self.frame, selectmode=EXTENDED,
                              width=80, height=1, fg="blue",
                              font=("Courier", 11, "bold"))
        self.header.insert(END,
   "Port        Module Type   Part Number      Serial Number    Manufacturer")
        self.header.pack()

        # the Finisar logo at the bottom of the screen
        logo = PhotoImage(file="Finisar_Logo.GIF")
        self.label = Label(self.frame, image=logo)
        self.label.image = logo  # kludge required by Tkinter
        self.label.pack(side=BOTTOM)

        # the one-liner instructing how to use this screen
        self.instr = Listbox(self.frame, selectmode=EXTENDED,
                             width=68, height=1, fg="blue",
                             font=("Courier", 9, ""))
        self.instr.insert(END,
    "  (highlight one or more ports, then right click to select action)")
        self.instr.pack(side=BOTTOM)

        # and the actual inventory of ports
        lines = len(self.portlist) + 5
        if lines > 28:
            lines = 28

        # build the scrollable listbox
        sb = Scrollbar(self.frame)
        sb.pack(side=RIGHT, fill=Y)
        self.inv = Listbox(self.frame, selectmode=EXTENDED,
                           width=80, height=lines, fg="black",
                           font=("Courier", 11, ""))
        self.inv.config(yscrollcommand=sb.set)
        sb.config(command=self.inv.yview)

        # and fill it in
        for port in self.portlist:
            self.inv.insert(END, portstr(port))
        self.inv.bind("<Double-Button-1>", self.invmenu_builder)
        self.inv.bind("<ButtonRelease-3>", self.invmenu_builder)
        self.inv.pack()

    def invmenu_builder(self, event):
        kill_widgets()
        if self.inv.curselection() == ():
            oopsbox = Listbox(self.frame, width=50, height=1)
            killlist.append(oopsbox)
            oopsbox.pack()
            oopsbox.insert(END,
                           "HIGHLIGHT one or more ports, THEN right click")
            return
        invmenu = Menu(self.inv, tearoff=0)
        killlist.append(invmenu)
        invmenu.add_command(label="Serial ID Keys", command=self.serial_id_h)
        invmenu.add_command(label="DOM Status", command=self.dom_h)
        invmenu.add_command(label="All Keys", command=self.all_h)
        invmenu.add_command(label="Code Sample", command=self.code_h)

        invmenu.post(event.x_root, event.y_root)

    def dom_h(self):
        self.showports("DOM")

    def serial_id_h(self):
        self.showports("SERIAL_ID")

    def all_h(self):
        self.showports("ALL")

    def code_h(self):
        self.showports("CODE")

    def showports(self, func):
        # get rid of any old windows hanging around
        kill_widgets()

        for portnum in self.inv.curselection():
            port = self.portlist[portnum]
            if port.port_type == 0:
                continue
            if func == "DOM":
                newroot = Tk()  # get a new window
                newroot.title("%s DOM Status" % port.port_name)

                # put a frame in it so it can scroll
                frame = Frame(newroot, bd=2, relief=SUNKEN)
                frame.pack()

                # put a canvas in it to hold all the graphics
                cwidth = 1000
                if port.port_type == 3:  # SFP gets a narrower canvas
                    cwidth = 550
                c = Canvas(frame, bd=0, width=cwidth, height=300,
                           scrollregion=(0, 0, 1540, 300))
                c.pack_propagate(0)  # don't let it autoscale
                c.pack(side=TOP, expand=True, fill=BOTH)

                # give the canvas a scrollbar
                dsbar = Scrollbar(frame, orient=HORIZONTAL)
                dsbar.pack(side=BOTTOM, fill=X)
                dsbar.config(command=c.xview)
                c.config(xscrollcommand=dsbar.set)
                draw_DOM(c, port)
                continue

            # Common code path for "CODE", "ALL", and any fmap

            if func == "CODE":
                boxheight = 24
            else:
                try:  # see if 'func' is a key in the function map (eg DOM)
                    keys = port.fmap[func]
                except:  # if not, special case "ALL keys"
                    if func == "ALL":
                        keys = []
                        for key in port.mmap:
                            keys.append(key)
                    else:
                        continue
                boxheight = len(keys)
            if boxheight > 40:
                boxheight = 40

            # build the scrollable listbox in a separate window
            newroot = Tk()
            newf = Frame(newroot, bd=2, relief=SUNKEN)
            newf.pack()
            sbar = Scrollbar(newf)
            sbar.pack(side=RIGHT, fill=Y)
            portbox = Listbox(newf, width=75, height=str(boxheight))
            portbox.pack()
            portbox.config(yscrollcommand=sbar.set)
            sbar.config(command=portbox.yview)

            # and add contents
            if func == 'CODE':
                printcode(newroot, portbox, port, portnum)
            else:
                newroot.title("%s keys -- %s" % (func, portstr(port)))

                # this code lifted from oomlib.py::oom_get_memory
                # specifically to read each page once, if it
                # holds dynamic keys (and don't read at all if all static)
                mm = port.mmap
                for key in keys:
                    if mm[key][0] == 1:         # Dynamic key, invalidate cache
                        pagekey = mm[key][3]    # page of interest
                        if mm[key][4] < 128:    # unless it's low memory page
                            pagekey = -1        # whose pagekey is -1
                        port.invalidate_page(mm[key][2], pagekey)

                for key in sorted(keys):
                    val = oom_get_keyvalue_cached(port, key)
                    decoder = port.mmap[key][1]
                    if ((decoder == 'get_bytes') or
                        (decoder == 'get_cablespec')):
                        valstr = get_hexstr(val)
                    else:
                        valstr = str(val)
                    endstr = '%s: %s' % (key, valstr)
                    portbox.insert(END, endstr)


#
# The main routine, start here :-)
# Get a root window, call the OOMdemo _init_ function, loop in Tkinter
#
root = Tk()
app = OOMdemo(root)
root.mainloop()
try:
    root.destroy()
except:
    pass
