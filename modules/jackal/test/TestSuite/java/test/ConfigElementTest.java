package test;

import java.util.*;
import junit.framework.*;
import org.vrjuggler.jccl.config.*;
import org.vrjuggler.jccl.config.event.*;

/**
 * Test the ConfigElement class.
 */
 public class ConfigElementTest
   extends TestCase
{
   public ConfigElementTest(String name)
   {
      super(name);
   }

   public void setUp()
   {
      // Create a simple chunk desc we can use for the tests
      mDesc = new ChunkDesc();
      mDesc.setName("Person");
      mDesc.setToken("person");
      // Name
      PropertyDesc prop = new PropertyDesc();
      prop.setName("Name");
      prop.setToken("name");
      mDesc.addPropertyDesc(prop);
      // Age
      prop = new PropertyDesc();
      prop.setName("Age");
      prop.setToken("age");
      prop.setValType(ValType.INT);
      mDesc.addPropertyDesc(prop);
      // Friends
      prop = new PropertyDesc();
      prop.setName("Friends");
      prop.setToken("friends");
      prop.setValType(ValType.EMBEDDEDCHUNK);
      prop.setHasVariableNumberOfValues(true);
      mDesc.addPropertyDesc(prop);
   }

   private ConfigElement makePerson(String name, int age, List friends)
   {
      Map props = new TreeMap();
      // name
      List prop_values = new ArrayList();
      prop_values.add(name);
      props.put("name", prop_values);
      // age
      prop_values = new ArrayList();
      prop_values.add(new Integer(age));
      props.put("age", prop_values);
      // friends
      props.put("friends", friends);

      return new ConfigElement(mDesc, name, props);
   }

   public void testConstructor()
   {
      List friends = new ArrayList();
      friends.add(makePerson("Allen", 27, new ArrayList()));
      friends.add(makePerson("Aron", 21, new ArrayList()));
      friends.add(makePerson("Ben", 22, new ArrayList()));
      friends.add(makePerson("Josh", 24, new ArrayList()));

      ConfigElement e = makePerson("Patrick", 27, friends);
      assertEquals(e.getDefinition(), mDesc);
      assertEquals(e.getName(), "Patrick");
      assertEquals(e.getProperty("name", 0), "Patrick");
      assertEquals(e.getProperty("age", 0), new Integer(27));
      for (int i=0; i<friends.size(); ++i)
      {
         assertEquals(e.getProperty("friends", i), friends.get(i));
      }
   }

   public void testSetName()
   {
      class Listener extends ConfigElementAdapter
      {
         public void nameChanged(ConfigElementEvent evt)
         {
            fired = true;
         }

         public boolean fired = false;
      }

      Listener l = new Listener();

      ConfigElement e = makePerson("Allen", 27, new ArrayList());
      e.addConfigElementListener(l);
      e.setName("Patrick");
      // Make sure the name was actually changed
      assertEquals(e.getName(), "Patrick");
      
      // Make sure an event was posted
      assertTrue(l.fired);
   }

   public void testGetProperty()
   {
      List friends = new ArrayList();
      friends.add(makePerson("Allen", 27, new ArrayList()));
      friends.add(makePerson("Aron", 21, new ArrayList()));
      ConfigElement e = makePerson("Patrick", 27, friends);

      assertEquals(e.getProperty("name", 0), "Patrick");
      assertEquals(e.getProperty("age", 0), new Integer(27));
      assertEquals(e.getProperty("friends", 0), makePerson("Allen", 27, new ArrayList()));
      assertEquals(e.getProperty("friends", 1), makePerson("Aron", 21, new ArrayList()));
   }

   public void testSetProperty()
   {
      class Listener extends ConfigElementAdapter
      {
         public void propertyValueChanged(ConfigElementEvent evt)
         {
            fired = true;
         }

         public boolean fired = false;
      }

      Listener l = new Listener();

      List friends = new ArrayList();
      friends.add(makePerson("Allen", 27, new ArrayList()));
      ConfigElement e = makePerson("Patrick", 27, friends);
      e.addConfigElementListener(l);

      e.setProperty("name", 0, "Aron");
      assertEquals(e.getProperty("name", 0), "Aron");
      assertTrue(l.fired);
      l.fired = false;

      e.setProperty("age", 0, new Integer(21));
      assertEquals(e.getProperty("age", 0), new Integer(21));
      assertTrue(l.fired);
      l.fired = false;

      e.setProperty("friends", 0, makePerson("Ben", 22, new ArrayList()));
      assertEquals(e.getProperty("friends", 0), makePerson("Ben", 22, new ArrayList()));
      assertTrue(l.fired);
   }

   public void testAddProperty()
   {
      class Listener extends ConfigElementAdapter
      {
         public void propertyValueAdded(ConfigElementEvent evt)
         {
            fired = true;
         }

         public boolean fired = false;
      }

      Listener l = new Listener();

      List friends = new ArrayList();
      friends.add(makePerson("Allen", 27, new ArrayList()));
      ConfigElement e = makePerson("Patrick", 27, friends);
      e.addConfigElementListener(l);

      e.addProperty("friends", makePerson("Josh", 24, new ArrayList()));
      assertEquals(e.getProperty("friends", 0), makePerson("Allen", 27, new ArrayList()));
      assertEquals(e.getProperty("friends", 1), makePerson("Josh", 24, new ArrayList()));
      assertTrue(l.fired);
   }

   public void testRemoveProperty()
   {
      class Listener extends ConfigElementAdapter
      {
         public void propertyValueRemoved(ConfigElementEvent evt)
         {
            fired = true;
         }

         public boolean fired = false;
      }

      Listener l = new Listener();

      List friends = new ArrayList();
      friends.add(makePerson("Allen", 27, new ArrayList()));
      friends.add(makePerson("Aron", 21, new ArrayList()));
      friends.add(makePerson("Josh", 24, new ArrayList()));
      ConfigElement e = makePerson("Patrick", 27, friends);
      e.addConfigElementListener(l);

      assertEquals(e.getPropertyValueCount("friends"), 3);
      e.removeProperty("friends", 0);
      assertEquals(e.getPropertyValueCount("friends"), 2);
      assertEquals(e.getProperty("friends", 0), makePerson("Aron", 21, new ArrayList()));
      assertEquals(e.getProperty("friends", 1), makePerson("Josh", 24, new ArrayList()));
      assertTrue(l.fired);
      l.fired = false;

      e.removeProperty("friends", 1);
      assertEquals(e.getPropertyValueCount("friends"), 1);
      assertEquals(e.getProperty("friends", 0), makePerson("Aron", 21, new ArrayList()));
      assertTrue(l.fired);
      l.fired = false;

      e.removeProperty("friends", 0);
      assertEquals(e.getPropertyValueCount("friends"), 0);
      assertTrue(l.fired);
   }

   private ChunkDesc mDesc;
}
