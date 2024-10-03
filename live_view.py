import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
import sqlite3
import time

fig = plt.figure()
ax1 = fig.add_subplot(1,1,1)
con = sqlite3.connect('./event_log.sqlite')
c = con.cursor()

def animate(i):
    c.execute("""
    select ms, Gbps, avg(Gbps) over(order by ms range between 10 PRECEDING and 10 FOLLOWING) as RA
    FROM
    (select _time / 83000000. * 1000. as ms,
    value * 8. * 8 * 1000. / 1024. / 1024. / 1024. as Gbps
     from GlobalLogger_usbReadsPerMs order by _key desc LIMIT 5000) as TMP
    """)
#     c.execute("""
#     select ms, MHz, avg(MHz) over(order by ms range between 10 PRECEDING and 10 FOLLOWING) as RA
#     FROM
#     (select _time / 83000000. * 1000. as ms,
#     0x20000 * 83000000. / value / 1000 / 1000 as MHz
#      from GlobalLogger_clk_output_cnt_0 order by _key desc LIMIT 5000) as TMP
#     """)

    data = c.fetchall()

    datas = []
    dates = []

    for row in data:
        datas.append(row[0])
        dates.append(float(row[1]))

    ax1.clear()
    ax1.plot(datas,dates,'-')

    plt.xlabel('ms')
    plt.ylabel('MHz')

import signal
signal.signal(signal.SIGINT, signal.SIG_DFL)

ani = animation.FuncAnimation(fig, animate, interval=100)
plt.show()
