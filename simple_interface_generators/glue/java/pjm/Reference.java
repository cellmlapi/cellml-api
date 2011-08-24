package pjm;

/**
 * Stores a reference to an object of the type of generic parameter T, which
 * is used to implement out and in/out parameters.
 */
public class Reference<T>
{
    /**
     * Sets up a reference with null as the initial referent.
     */
    public Reference()
    {
        mObject = null;
    }

    /**
     * Sets up a reference with aObject as the initial referent.
     * @param aObject The initial referent.
     */
    public Reference(T aObject)
    {
        mObject = aObject;
    }

    /**
     * Fetches the referent.
     * @return The referent.
     */
    public T get()
    {
        return mObject;
    }

    /**
     * Sets the referent.
     * @param aObject The new referent.
     */
    public void set(T aObject)
    {
        mObject = aObject;
    }

    private T mObject;
};
