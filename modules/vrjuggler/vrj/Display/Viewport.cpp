#include <vrj/vrjConfig.h>

#include <vrj/Display/Viewport.h>
#include <vrj/Kernel/Kernel.h>

namespace vrj
{

void Viewport::config(jccl::ConfigChunkPtr chunk)
{
   vprASSERT(chunk.get() != NULL);

   // -- Get config info from chunk -- //
    float originX = chunk->getProperty("origin", 0);
    float originY = chunk->getProperty("origin", 1);
    float sizeX   = chunk->getProperty("size", 0);
    float sizeY   = chunk->getProperty("size", 1);
    std::string name  = chunk->getProperty("name");
    mView    = (Viewport::View)(int)chunk->getProperty("view");
    mActive  = chunk->getProperty("active");

   // -- Check for error in configuration -- //
   // NOTE: If there are errors, set them to some default value
   if(sizeX <= 0)
   {
      vprDEBUG(vrjDBG_DISP_MGR,2) << "WARNING: viewport sizeX set to: " << sizeX
                                << ".  Setting to 1." << std::endl
                                << vprDEBUG_FLUSH;
      sizeX = 1.0f;
   }

   if(sizeY <= 0)
   {
      vprDEBUG(vrjDBG_DISP_MGR,2) << "WARNING: viewport sizeY set to: " << sizeY
                                << ".  Setting to 1." << std::endl
                                << vprDEBUG_FLUSH;
      sizeY = 1.0f;
   }

    // -- Set local window attributes --- //
    setOriginAndSize(originX, originY, sizeX, sizeY);

    // Get the user for this display
    std::string user_name = chunk->getProperty("user");
    mUser = Kernel::instance()->getUser(user_name);

    if(NULL == mUser)
    {
       vprDEBUG(vprDBG_ERROR,0) << clrOutNORM(clrRED, "ERROR:") << " User not found named: "
                              << user_name.c_str() << std::endl
                              << vprDEBUG_FLUSH;
      vprASSERT(false && "User not found in Viewport::config");
    }

    setName(name);
    mViewportChunk = chunk;        // Save the chunk for later use

    std::string bufname = "Head Latency " + name;
    mLatencyMeasure.setName (bufname);
}

std::ostream& operator<<(std::ostream& out, Viewport& viewport)
{
   return viewport.outStream(out);
}

std::ostream& Viewport::outStream(std::ostream& out)
{
   out << "user: " << getName()
       << "  org:" << mXorigin << ", " << mYorigin
       << "  sz:" << mXsize << ", " << mYsize
       << "  view:" << ((mView == Viewport::LEFT_EYE) ? "Left" : ((mView==Viewport::RIGHT_EYE)?"Right" : "Stereo") )
       << std::flush;

   return out;
}


};
