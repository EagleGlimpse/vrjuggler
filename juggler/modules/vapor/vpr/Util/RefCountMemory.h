#ifndef VPR_REFERENCE_COUNTABLE_MEMORY
#define VPR_REFERENCE_COUNTABLE_MEMORY
#include <assert.h>


namespace vpr
{
   //: reference countable memory. (no, not radio controlled, sorry)
   //
   // To use reference counting on your objects, inherit from this class.
   //
   // NOTES:
   // - you MUST call unrefDelete or checkDelete to delete, do not call 
   //   operator::delete() yourself!!!!!
   // - on creation, refcount == 0, so if you intend to own the memory 
   //   created, you should call ref() on the object
   //
   // EXAMPLE USAGE:
   //   Object obj;
   //   obj.ref();  // count == 1;
   //   obj.ref();  // count == 2;
   //   int count = obj.getRef();
   //   obj.unrefDelete(); // count == 1, memory is not deleted
   //   obj.unrefDelete(); // count == 0, memory is deleted
   // 
   // class Object : public vpr::RefCountMemory
   class RefCountMemory
   {
   public:
      enum States
      {
         BAD = -69,
      };
       //: default constructor
       // on creation, refcount == 0, so if you intend to own the memory 
       // created, you should ref it
      RefCountMemory();

       // on creation, refcount == 0, so if you intend to own the memory 
       // created, you should ref it
      RefCountMemory( const RefCountMemory& r ) : ___mNumTimesReferenced( 0 )
       {
          //dont copy ref number.
       }

   protected:
       //: destructor
       // you MUST call checkDelete to delete, do not call delete!!!!!
       virtual ~RefCountMemory()
       {
          assert( ___mNumTimesReferenced != BAD && ___mNumTimesReferenced >= 0 && "this data has been dereferenced more times than referenced, someone is probably holding on to the data after they called deref(), or someone called deref too many times. also, make sure you use checkDelete, or unrefDelete." );
          ___mNumTimesReferenced = BAD;
       }

   public:
       RefCountMemory& operator=( const RefCountMemory& r )
       {
          //dont copy ref number.
          return *this;
       }

      //: increase the reference count by 1, return the new refcount
      const int& ref();

      //: decrease the reference count by 1, then call checkDelete
      // equivalent to calling RefCountMemory::unref followed by RefCountMemory::checkDelete
      // return true if deleted, false if not
      bool unrefDelete();

   public:
      //: decrease the reference count by 1, return the new refcount
      const int& unref();

      //: return the reference count
      const int& getRef() const;

      //: decrease the reference count by 1, return the new refcount
      const int& unrefGetRef();

      //: if refcount <= 0, then delete the memory
      //  if the object has any refcountable children, then they will
      //  have RefCountMemory::unrefDelete() called on them.
      // return true if deleted, false if not
      virtual bool checkDelete();

   private:
      mutable int ___mNumTimesReferenced;
   };

   //: default constructor
   inline RefCountMemory::RefCountMemory() : ___mNumTimesReferenced( 0 )
   {
   }

   //: get the reference count
   inline const int& RefCountMemory::getRef() const
   {
      assert( ___mNumTimesReferenced != BAD && "this data has been deleted, someone is probably holding on to the data after they called deref()" );
      return ___mNumTimesReferenced;
   }
   
   //: increase the reference count
   inline const int& RefCountMemory::ref()
   {
      assert( ___mNumTimesReferenced != BAD && "this data has been deleted, someone is probably holding on to the data after they called deref()" );
      ++___mNumTimesReferenced;
      return this->getRef();
   }

   //: decrease the reference count.
   inline const int& RefCountMemory::unref()
   {
      assert( ___mNumTimesReferenced != BAD && ___mNumTimesReferenced >= 0 && "this data has been dereferenced more times than referenced, someone is probably holding on to the data after they called deref(), or someone called deref too many times. also, make sure you use checkDelete, or unrefDelete." );
      --___mNumTimesReferenced;
      return this->getRef();
   }

   //: if refcount <= 0, then delete the memory
   //  if the object has any refcountable children, then they will
   //  have RefCountMemory::unrefDelete() called on them.
   // return true if deleted, false if not
   inline bool RefCountMemory::checkDelete()
   {
      assert( ___mNumTimesReferenced != BAD && ___mNumTimesReferenced >= 0 && "this data has been dereferenced more times than referenced, someone is probably holding on to the data after they called deref(), or someone called deref too many times. also, make sure you use checkDelete, or unrefDelete." );
      if (___mNumTimesReferenced <= 0)
      {
         delete this;
         return true;
      }
      else
      {
         return false;
      }   
   }
   
   //: decrease the reference count by 1, then call checkDelete
   // equivalent to calling RefCountMemory::unref followed by RefCountMemory::checkDelete
   // return true if deleted, false if not
   inline bool RefCountMemory::unrefDelete()
   {
      this->unref();
      return this->checkDelete();
   }

   //: decrease the reference count by 1, return the new refcount
   inline const int& RefCountMemory::unrefGetRef()
   {
      this->unref();
      return this->getRef();
   }
   
}; //namespace vpr

#endif
