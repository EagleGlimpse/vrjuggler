/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998, 1999, 2000 by Iowa State University
 *
 * Original Authors:
 *   Allen Bierbaum, Christopher Just,
 *   Patrick Hartling, Kevin Meinert,
 *   Carolina Cruz-Neira, Albert Baker
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date$
 * Version:       $Revision$
 * -----------------------------------------------------------------
 *
 *************** <auto-copyright.pl END do not edit this line> ***************/


#include <jccl/Plugins/PerformanceMonitor/PerfDataBuffer.h>
//#include <vpr/Util/Debug.h>

namespace jccl {

PerfDataBuffer::PerfDataBuffer (const std::string& _name, int _numbufs,
                                int _nindex) {
    name = _name;
    numbufs = _numbufs;
    buffer = new buf_entry[numbufs];
    read_begin = 0;
    write_pos = 1;
    lost = 0;
    active = false;
    nindex = _nindex;
}



PerfDataBuffer::~PerfDataBuffer () {
    active = false;
    delete[] buffer;
}



void PerfDataBuffer::activate() {
    active = true;
    vprDEBUG(jcclDBG_PERFORMANCE,1) << "Performance Buffer " << name << 
	" activated.\n" << vprDEBUG_FLUSH;
}



void PerfDataBuffer::deactivate() {
    active = false;
    /* deactivate maybe should reset the buffer so it's reactivated
     * with a clean slate, thusly:
     */
    read_begin = 0;
    write_pos = 1;
    lost = 0;
    vprDEBUG(jcclDBG_PERFORMANCE,1) << "Performance Buffer " << name << 
	" deactivated.\n" << vprDEBUG_FLUSH;

}



bool PerfDataBuffer::isActive() const {
    return active;
}



void PerfDataBuffer::set(int _phase) {
    int tw;

    if (!active)
	return;

    if (write_pos == read_begin) {
	if (lost_lock.acquire() != -1) {
	    lost++;
	    lost_lock.release();
	}
	else
	    vprDEBUG(vprDBG_ALL,2) << "PerfDataBuffer: lock acquire "
		       << "failed\n" << vprDEBUG_FLUSH;
	tw = (write_pos + numbufs - 1) % numbufs;
	buffer[tw].phase = _phase;
	buffer[tw].ts.set();
    }
    else {
	buffer[write_pos].phase = _phase;
	buffer[write_pos].ts.set();
	write_pos = (write_pos+1)%numbufs;
    }
}


void PerfDataBuffer::set (int _phase, TimeStamp& _value) {
    int tw;

    if (!active)
	return;

    if (write_pos == read_begin) {
	lost_lock.acquire();
        lost++;
        lost_lock.release();

        // overwrite last buffer pos with most recent
	tw = (write_pos + numbufs - 1) % numbufs;
	buffer[tw].phase = _phase;
	buffer[tw].ts = _value;
    }
    else {
	buffer[write_pos].phase = _phase;
	buffer[write_pos].ts = _value;
	write_pos = (write_pos+1)%numbufs;
    }
}



void PerfDataBuffer::write (std::ostream& out) {
    // the only tricky part of this is that the region we
    // want to print out might wrap back around to the
    // beginning of the list.  That's what the 2nd big
    // case is for.
    int begin, end, i, tlost;
    buf_entry* b;

    //out.width(13);

    if (!active)
	return;

    //out << "PerfData " << name << "\n";
    begin = read_begin;
    end = (write_pos - 1 + numbufs)%numbufs;
    //cout << "begin/end are " << begin <<' '<< end << endl;
    if (begin == end)
	return;
    out << "PerfData1 \"" << name << "\" " << nindex << std::endl;
    if (begin < end) {
	for (i = begin; i < end; i++) {
	    b = &(buffer[i]);
	    out << b->phase << ' '
		<< std::setiosflags(std::ios::fixed) << b->ts << '\n';
	}
    }
    else { /* wraparound */
	for (i = begin; i < numbufs; i++) {
	    b = &(buffer[i]);
	    out << b->phase << ' ' << std::setiosflags(std::ios::fixed)
		<< b->ts << '\n';
	}
	for (i = 0; i < end; i++) {
	    b = &(buffer[i]);
	    out << b->phase << ' ' << std::setiosflags(std::ios::fixed)
		<< b->ts << '\n';
	}
    }
	
    lost_lock.acquire();
    tlost = lost;
    lost = 0;
    lost_lock.release();
    read_begin = end;

    out << -1 << ' ' << tlost << std::endl;

}



// this probably isn't safe in a multitasking environment
void PerfDataBuffer::writeTotal(std::ostream& out, int preskip, int postskip, float discrep) {
    int begin = read_begin;
    int end = (write_pos - 1 + numbufs)%numbufs;
    int last = (end + numbufs-1) %numbufs;
    float retval;
    int nump;
    int i;
    float diff, avg, x;

    retval = buffer[(end-postskip)%numbufs].ts - buffer[(begin+preskip)%numbufs].ts;
    /*
    cout << "begin is " << begin
	 << "\nend is " << end
	 << "\nlast is " << last << endl;
    */
    if (last > begin)
	nump = last - begin;
    else
	nump = last + (numbufs - begin);

    avg = retval/(nump - preskip - postskip);

    out << "Dumping buffer: " << nump << " samples, total: "
	<< retval << " us, avg: " << avg << "us\n";
    if (discrep > 0) {
	out << "Reporting Discrepencies:\n";
    }
    end = (end-postskip)%numbufs;
    begin = (begin + preskip)%numbufs;

	if (begin == end)
	    return;
	else if (begin < end) {
	    for (i = begin+1; i < end; i++) {
		diff = buffer[i].ts - buffer[i-1].ts;
                x = diff - avg;
                if (x < 0)
                    x = -x;
		if (x > avg * discrep) {
		    out << "    " << diff << " us at time "
			<< buffer[i-1].ts << " us\n";
		}
	    }
	}
	else { /* wraparound */
	    for (i = begin+1; i < numbufs; i++) {
		diff = buffer[i].ts - buffer[i-1].ts;
                x = diff - avg;
                if (x < 0)
                    x = -x;
		if (x > avg * discrep) {
		    out << "    " << diff << " us at time "
			<< buffer[i-1].ts << " us\n";

		}
	    }
	    diff = buffer[0].ts - buffer[numbufs].ts;
            x = diff - avg;
            if (x < 0)
                x = -x;
	    if (x > avg * discrep) {
		out << "    " << diff << " us at time "
		    << buffer[numbufs].ts << " us\n";
	    }
	    for (i = 1; i < end; i++) {
		diff = buffer[i].ts - buffer[i-1].ts;
                x = diff - avg;
                if (x < 0)
                    x = -x;
		if (x > avg * discrep) {
		    out << "    " << diff << " us at time "
			<< buffer[i-1].ts << " us\n";
		}
	    }
	
	}


    lost_lock.acquire();
    lost = 0;
    lost_lock.release();
    read_begin = end;

}


void PerfDataBuffer::clear () {

    lost_lock.acquire();
    read_begin = 0;
    write_pos = 1;
    lost = 0;
    lost_lock.release();

}

};
