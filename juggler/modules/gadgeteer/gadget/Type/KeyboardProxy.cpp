
#include <vjConfig.h>
#include <Kernel/vjKernel.h>
#include <Input/InputManager/vjKeyboardProxy.h>

bool vjKeyboardProxy::config(vjConfigChunk* chunk)
{
   vjDEBUG_BEGIN(vjDBG_INPUT_MGR,3) << "------------------ KEYBOARD PROXY config()-----------------\n" << vjDEBUG_FLUSH;
   vjASSERT(((std::string)chunk->getType()) == "KeyboardProxy");

   int unitNum = chunk->getProperty("unit");
   std::string proxy_name = chunk->getProperty("name");
   std::string dev_name = chunk->getProperty("device");

   int proxy_num = vjKernel::instance()->getInputManager()->addKeyboardProxy(dev_name,unitNum,proxy_name,this);

   if ( proxy_num != -1)
   {
      vjDEBUG_END(vjDBG_INPUT_MGR,3) << "   KeyboardProxy config()'ed" << endl << vjDEBUG_FLUSH;
      return true;
   }
   else
   {
      vjDEBUG(vjDBG_INPUT_MGR,0) << "   KeyboardProxy config() failed" << endl << vjDEBUG_FLUSH;
      vjDEBUG_END(vjDBG_INPUT_MGR,3) << endl << vjDEBUG_FLUSH;
      return false;
   }
}
