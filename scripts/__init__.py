import CoreServer
import Frontend
#import World
import SAMP

THE_SPAWN_COORDS = [0,0,10] #[1529.6,-1691.2,13.3]

#alle entities extend from this
class GenericEntity(CoreServer.BaseEntity):
	def __init__(self):
		print("base entity")
		self.IsBot = False

class BotEntity(GenericEntity):
	def __init__(self):
		super(BotEntity, self).__init__()
		self.IsBot = True
		self.Name = "TheBot"
		self.Model = 111
		self.Health = 11.0
		self.Position = THE_SPAWN_COORDS
		self.World = 0
		self.Stream_Index = 0
		print("New bot entering server!")

class PlayerEntity(GenericEntity):
	def __init__(self, connection):
		super(PlayerEntity, self).__init__()

		self.connection = connection
		print("Make SAMPEntity: {}".format(connection))
	def OnEnterWorld(self):
		print("Enter world!")
		self.connection.SendMessage(0xFFFFFFFF, ("Welcome to the Python server, your IP is: {}:{}".format(self.connection.GetIP(), self.connection.GetPort())))
	def handle_spawncmd(self, string):
		self.Spawn(
			{
				'position': THE_SPAWN_COORDS,
				'model': 111
			})
class SAMP3DTextEntity(GenericEntity):
	def __init__(self):
		print("asdasd")

class VehicleEntity(GenericEntity):
	def __init__(self):
		print("new veh entity")	

class PickupEntity(GenericEntity):
	def __init__(self):
		print("Pickup entity")
class SAMPHandler(CoreServer.Connection, CoreServer.CommandHandler):
	def handle_pmcmd(self, string):
		print("PM command: {}".format(string))
		self.Entity.Spawn({'position': [100.0,100.0,100.0], 'model': 51})
		self.SendMessage(0xFFFFFFFF, string)
	def handle_dialog(self, list_index, button_id, input):
		print("got dlg resp: {} {} ".format(self, input))
		self.SendMessage(0xFF00FF00, ("Dialog resp {} | {} | {}".format(list_index, button_id, input)))
	def handle_testcmd(self, string):
		self.Entity.Health = 50.0
		self.Entity.Armour = 50.0
		#self.Entity.Position = [0.0,0.0,100.0]
		#self.Entity.Weapons = {24: 50, 31: 1000}
		#self.Entity.Weapons[24] += 100
		#self.pickup = SAMP.CreatePickup({'model': 1222, 'position': [1529.6,-1691.2,13.3], 'pickup_type': 1, 'world': 0, 'stream_index': 0, 'pickup_event': pickup_event})
		self.Entity.Model = 6
		self._3dtext = SAMP.Create3DTextLabel({'position': THE_SPAWN_COORDS, 'text': 'This is some text', 'test_los': False, 'draw_distance': 500.0})
		self.vehicle = SAMP.CreateVehicle({'model':411, 'position': THE_SPAWN_COORDS, 'world': 0, 'stream_index': 0, 'colour': [1,0]})
		#World.CreateVehicle({'position': [1529.6,-1691.2,13.3], 'model': 411})
		self.SendMessage(0xFF00FFFF, "Set stuff")
	def handle_testcmd2(self, string):
		#SAMP.DestroyPickup(self.pickup)
		self.Entity.PutInVehicle(self.vehicle)
		SAMP.Destroy3DTextLabel(self._3dtext)
		self.SendMessage(0xFF00FFFF, "Pickup destrsoyed")
	def handle_testdlg(self, string):
		Frontend.CreateModal(self, {'title': 'Test Dialog', 'message': 'Please enter a string', 'type': Frontend.INPUT_BOX, 'buttons': ['Ok', 'Cancel'], 'callback': self.handle_dialog, 'extra': 'anything'})
	def __init__(self):
		self.Entity = PlayerEntity(self)
		self.SetEntity(self.Entity)
		#self.Entity.Camera.Position = [100, 100, 100]
		#self.Entity.Camera.Lerp(100.0, 200.0, 300.0, 18000)
		#Entity.Camera.Slerp()
		#self.PutInWorld(0)
		#self.SetStreamIndex(0)
		
		self.Entity.Health = 100
		samphandler_command_table = [
			{'primary_command': 'pm', 'aliases': ['privatemessage', 'privmsg'], 'function': self.handle_pmcmd},
			{'primary_command': 'dialog', 'aliases': ['dlg'], 'function': self.handle_testdlg},
			{'primary_command': 'test', 'aliases': ['blah'], 'function': self.handle_testcmd},
			{'primary_command': 'boom', 'aliases': ['blah'], 'function': self.handle_testcmd2},
			{'primary_command': 'spawn', 'function': self.Entity.handle_spawncmd},

		] #commands which are usable at any user state
		print("new connection {}, IP {}:{}".format(self,self.GetIP(),self.GetPort()))
		self.RegisterCommands(samphandler_command_table)

def pickup_event(pickup_entity, player_entity):
	print("asdasd")
CoreServer.SetConnectionHandler(SAMPHandler)
SAMP.SetPickupEntity(PickupEntity)
SAMP.Set3DTextLabelEntity(SAMP3DTextEntity)
SAMP.SetVehicleEntity(VehicleEntity)
#SAMP.CreatePickup({'model': 1222, 'position': [1529.6,-1691.2,13.3], 'pickup_type': 1, 'world': 0, 'stream_index': 0, 'pickup_event': pickup_event})
#SAMP.Create3DTextLabel({'text': 'Some text','position': [1529.6,-1691.2,15.3],'colour': 0xFFFFFFFF, 'world': 0, 'stream_index': 0});
#SAMP.CreateVehicle({'model':411, 'position': [1529.6,-1691.2,13.3], 'world': 0, 'stream_index': 0, 'colour': [1,0]})

the_bot = BotEntity()
CoreServer.AddEntity(the_bot)

#the_bot.RemoveEntity(the_bot)