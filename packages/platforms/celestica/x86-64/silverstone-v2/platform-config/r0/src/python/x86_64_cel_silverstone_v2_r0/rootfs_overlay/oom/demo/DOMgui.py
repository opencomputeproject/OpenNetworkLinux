#
# DOMgui.py - draws the DOM display for the OOM graphical demo
# Copyright 2017 Finisar Inc.
# Author: Don Bollinger don@thebollingers.org
#
from oom import *
from Tkinter import *


def yscale(ptop, pbot, vmin, vmax, vact):
    prange = pbot - ptop
    vrange = vmax - vmin
    voffs = vact - vmin
    p = pbot - (float(voffs * prange) / vrange)
    return p


def draw_hotscale(master, stype, channel, h_al, h_wa, l_wa, l_al,
                  actual, aw, DOMclass):

    # parms:   ID     max, min, minsc, step, label, unit, format
    parms = {'TEMP': (130, -50, -40, 20, "Temperature", "C", "%.3f"),
             'VCC':  (6.6, 0, 0, 1, "Vcc  3.3V", "V", "%.3f"),
             'BIAS': (25, 0, 0, 10, "Laser Bias", "mA", "%.3f"),
             'TX_P': (5, -30, -30, 5, "Tx Power", "dbm", "%.2f"),
             'RX_P': (5, -30, -30, 5, "Rx Power", "dbm", "%.2f")
             }

    vmax, vmin, minsc, step, label, unit, formstr = parms[stype]
    if (label == "dbm") and (actual < vmin):
        actual = vmin
    if channel > 0:
        label += "(%d)" % channel
    if (h_al > vmax):   # high alarm exceeds scale, make it bigger!
        vmax = h_al * 1.2

    cwidth = 110
    c = Canvas(master, width=cwidth, height=300,
               bd=1, highlightbackground="grey")
    # c.pack(side=LEFT)
    master.create_window(DOMclass.leftedge, 0, window=c, anchor=NW)
    DOMclass.leftedge += cwidth

    left = 30
    width = 20
    right = left + width
    top = 25
    height = 250
    bottom = height + top

    # Label this graphic
    c.create_text(5, 4, anchor=NW, font=("Calibri", 9, "bold"), text=label)
    c.create_text(right+28, top+80,
                  font=("Calibri", 13, "bold"), text=unit)
    c.create_text(right+28, top+105,
                  font=("Calibri", 11, "bold"), text=formstr % actual)
    # Add the scale
    lleft = left - 3
    scval = minsc
    while scval <= vmax:
        y = yscale(top, bottom, vmin, vmax, scval) - 7
        t = str(scval)
        c.create_text(lleft, y, anchor=NE, font=("Calibri", 8, "bold"), text=t)
        scval += step

    # build the red/yellow/green graphic
    # note here, the top of the warning (watop) is
    # defined by "high alarm" (h_al), similarly for all 4 boundaries
    watop = yscale(top, bottom, vmin, vmax, h_al)
    wabot = yscale(top, bottom, vmin, vmax, l_al)
    oktop = yscale(top, bottom, vmin, vmax, h_wa)
    okbot = yscale(top, bottom, vmin, vmax, l_wa)

    # x_offs, y_offs, right, height
    r = c.create_rectangle(left, top, right, bottom, fill="red", outline="red")
    r = c.create_rectangle(left, watop, right, wabot,
                           fill="yellow", outline="yellow")
    r = c.create_rectangle(left, oktop, right, okbot,
                           fill="green", outline="green")

    pact = yscale(top, bottom, vmin, vmax, actual)
    r = c.create_polygon(right, pact-3, left, pact,
                         right, pact+3, fill="blue", outline="blue")

    # print alarms and warning
    ctextbox(c, "Alarm", formstr, h_al, 6, (aw & 8))
    ctextbox(c, "Warn", formstr, h_wa, 44, (aw & 2))
    ctextbox(c, "Warn", formstr, l_wa, 180, (aw & 1))
    ctextbox(c, "Alarm", formstr, l_al, 220, (aw & 4))


def ctextbox(c, label, formstr, value, position, active):

    awleft = 78
    top = 25
    height = 250

    if active != 0:
        color = "red"
    else:
        color = "white"

    c.create_text(awleft, top + position - 6,
                  font=("Calibri", 8, ""), text=label)
    c.create_rectangle(awleft-20, top + position,
                       awleft+20, top + position + 14,
                       fill=color, outline=color)
    c.create_text(awleft, top + position + 7,
                  font=("Calibri", 8, ""), text=formstr % value)


class draw_DOM:
    def __init__(self, root, port):

        self.leftedge = 0   # reset where the pictures go

        # Get all the key values first, before clearing the previous
        # screen (getting key values is slow, screen goes blank a long time)
        # note that the thresholds are cached, they are cheap to get
        # the actual current values are dynamic,
        # read from the module each time through

        typeSFP = 1 if (port.port_type == 0x3) or \
                       (port.port_type == 0xB) else 0
        typeQSFP = 1 if (port.port_type == 0xD) or \
                        (port.port_type == 0x11) else 0

        # invalidate the DOM page cache to get fresh data
        if typeSFP:
            port.invalidate_page(0xA2, -1)
        if typeQSFP:
            port.invalidate_page(0xA0, -1)

        # Get the alarm and warning boundaries
        tha = oom_get_keyvalue(port, 'TEMP_HIGH_ALARM')
        thw = oom_get_keyvalue(port, 'TEMP_HIGH_WARN')
        tla = oom_get_keyvalue(port, 'TEMP_LOW_ALARM')
        tlw = oom_get_keyvalue(port, 'TEMP_LOW_WARN')
        vha = oom_get_keyvalue(port, 'VOLTAGE_HIGH_ALARM')
        vhw = oom_get_keyvalue(port, 'VOLTAGE_HIGH_WARN')
        vla = oom_get_keyvalue(port, 'VOLTAGE_LOW_ALARM')
        vlw = oom_get_keyvalue(port, 'VOLTAGE_LOW_WARN')
        bha = oom_get_keyvalue(port, 'BIAS_HIGH_ALARM')
        bhw = oom_get_keyvalue(port, 'BIAS_HIGH_WARN')
        bla = oom_get_keyvalue(port, 'BIAS_LOW_ALARM')
        blw = oom_get_keyvalue(port, 'BIAS_LOW_WARN')
        tpha = oom_get_keyvalue(port, 'TX_POWER_HIGH_ALARM')
        tphw = oom_get_keyvalue(port, 'TX_POWER_HIGH_WARN')
        tpla = oom_get_keyvalue(port, 'TX_POWER_LOW_ALARM')
        tplw = oom_get_keyvalue(port, 'TX_POWER_LOW_WARN')
        rpha = oom_get_keyvalue(port, 'RX_POWER_HIGH_ALARM')
        rphw = oom_get_keyvalue(port, 'RX_POWER_HIGH_WARN')
        rpla = oom_get_keyvalue(port, 'RX_POWER_LOW_ALARM')
        rplw = oom_get_keyvalue(port, 'RX_POWER_LOW_WARN')

        # get the current values
        atemp = oom_get_keyvalue_cached(port, 'TEMPERATURE')
        if typeSFP:
            avolt = oom_get_keyvalue_cached(port, 'VCC')
            # taw: temp high alarm set, low alarm set,
            # high warn set, low warn set (4 bits)
            ta = oom_get_keyvalue_cached(port, 'L_TEMP_ALARM')
            tw = oom_get_keyvalue_cached(port, 'L_TEMP_WARN')
            taw = (ta * 4) + tw
            va = oom_get_keyvalue_cached(port, 'L_VCC_ALARM')
            vw = oom_get_keyvalue_cached(port, 'L_VCC_WARN')
            vaw = (va * 4) + vw
            abias = oom_get_keyvalue_cached(port, 'TX_BIAS')
            ba = oom_get_keyvalue_cached(port, 'L_BIAS_ALARM')
            bw = oom_get_keyvalue_cached(port, 'L_BIAS_WARN')
            baw = (ba * 4) + bw
            atxp = oom_get_keyvalue_cached(port, 'TX_POWER_DBM')
            tpa = oom_get_keyvalue_cached(port, 'L_TX_POWER_ALARM')
            tpw = oom_get_keyvalue_cached(port, 'L_TX_POWER_WARN')
            tpaw = (tpa * 4) + tpw
            arxp = oom_get_keyvalue_cached(port, 'RX_POWER_DBM')
            rpa = oom_get_keyvalue_cached(port, 'L_RX_POWER_ALARM')
            rpw = oom_get_keyvalue_cached(port, 'L_RX_POWER_WARN')
            rpaw = (rpa * 4) + rpw
        if typeQSFP:
            avolt = oom_get_keyvalue_cached(port, 'SUPPLY_VOLTAGE')
            taw = oom_get_keyvalue_cached(port, 'L_TEMP_ALARM_WARN')
            vaw = oom_get_keyvalue_cached(port, 'L_VCC_ALARM_WARN')
            abias = [0] * 5
            atxp = [0] * 5
            arxp = [0] * 5
            baw = [0] * 5
            tpaw = [0] * 5
            rpaw = [0] * 5
            for i in range(1, 5):
                abias[i] = oom_get_keyvalue_cached(port, 'TX%d_BIAS' % i)
                atxp[i] = oom_get_keyvalue_cached(port, 'TX%d_POWER_DBM' % i)
                arxp[i] = oom_get_keyvalue_cached(port, 'RX%d_POWER_DBM' % i)
                baw[i] = oom_get_keyvalue_cached(port, 'L_TX%d_BIAS' % i)
                tpaw[i] = oom_get_keyvalue_cached(port, 'L_TX%d_POWER' % i)
                rpaw[i] = oom_get_keyvalue_cached(port, 'L_RX%d_POWER' % i)

        # now, in case I've been here before, clear the screen
        for widget in root.winfo_children():
            widget.destroy()
        # root.title("%s DOM Status" % port.port_name)

        # and draw the graphs
        draw_hotscale(root, "TEMP", 0, tha, thw, tlw, tla, atemp, taw,
                      self)
        draw_hotscale(root, "VCC", 0, vha, vhw, vlw, vla, avolt, vaw,
                      self)

        if typeSFP:
            draw_hotscale(root, "BIAS", 0, bha, bhw, blw, bla, abias, baw,
                          self)
            draw_hotscale(root, "RX_P", 0, rpha, rphw, rplw, rpla, arxp, rpaw,
                          self)
            draw_hotscale(root, "TX_P", 0, tpha, tphw, tplw, tpla, atxp, tpaw,
                          self)
        if typeQSFP:
            for i in range(1, 5):
                draw_hotscale(root, "BIAS", i, bha, bhw, blw, bla,
                              abias[i], baw[i], self)
            for i in range(1, 5):
                draw_hotscale(root, "RX_P", i, rpha, rphw, rplw, rpla,
                              arxp[i], rpaw[i], self)
            for i in range(1, 5):
                draw_hotscale(root, "TX_P", i, tpha, tphw, tplw, tpla,
                              atxp[i], tpaw[i], self)

        # translation: for the root window, after 3000 ms, call self again
        root.after(3000, draw_DOM, root, port)

if __name__ == "__main__":
    portlist = oom_get_portlist()
    port = portlist[0]
    print(oom_get_keyvalue(port, "VENDOR_PN"))

    root = Tk()
    draw_DOM(root, port)
    root.mainloop()
