
package VjConfig;

import java.util.List;
import java.util.ArrayList;

public class ConfigIOStatus {

    List messages;  // either Strings or Throwables.
    int status;
    
    public static final int SUCCESS = 1;
    public static final int WARNINGS = 2;
    public static final int ERRORS = 3;   // loaded, but there were errors
    public static final int FAILURE = 4;  // total failure; nothing loaded

    public ConfigIOStatus () {
        messages = new ArrayList();
        clear();
    }

    public void clear () {
        messages.clear();
        status = SUCCESS;   
    }

    public void setStatus (int new_status) {
        // wish i had an assert

        // things can only go downhill from here
        if (new_status > status)
            status = new_status;
    }

    public int getStatus () {
        return status;
    }
    
//     public void addMessage (String msg) {
//         messages.add (msg);
//     }

    public void addWarning (String msg) {
        setStatus (WARNINGS);
        messages.add (msg);
    }

    public void addError (String msg) {
        setStatus (ERRORS);
        messages.add (msg);
    }

    public void addFailure (String msg) {
        setStatus (FAILURE);
        messages.add (msg);
    }

    public void addWarning (Throwable msg) {
        setStatus (WARNINGS);
        messages.add (msg);
    }

    public void addError (Throwable msg) {
        setStatus (ERRORS);
        messages.add (msg);
    }

    public void addFailure (Throwable msg) {
        setStatus (FAILURE);
        messages.add (msg);
    }

    public String toString () {
        StringBuffer s = new StringBuffer();
        switch (status) {
        case SUCCESS:
            s.append ("Completed succesfully.\n");
            break;
        case WARNINGS:
            s.append ("Completed with warnings.\n");
            break;
        case ERRORS:
            s.append ("Completed with errors.\n");
            break;
        case FAILURE:
            s.append ("Failed.\n");
            break;
        }
        for (int i = 0; i < messages.size(); i++) {
            Object o = messages.get(i);
            s.append (o.toString());
        }

        return new String(s);
    }

}

