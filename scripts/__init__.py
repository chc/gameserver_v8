import CoreServer
import Frontend
import SAMP

THE_SPAWN_COORDS = [1529.6,-1691.2,13.3]
CLASS_TYPE_ZOMBIE = 0
CLASS_TYPE_HUMAN = 1

TEAM_HUMAN = 0
TEAM_ZOMBIE = 1

DEFAULT_GRAVITY = 0.08

ZOMBIE_CLASSES = [
	{'skin': 4, 'name': 'Fast Joe', 'gravity': 0.005, 'CJ_run': True, 'description': 'Fast Joe is a fast zombie, who can jump high', 'contact_instant_zombie': False, 'health': 300.0},
	{'skin': 5, 'name': 'Slow Sal', 'gravity': 0.010, 'CJ_run': False, 'description': 'Fast Joe is a fast zombie, who can jump high', 'contact_instant_zombie': False, 'health': 800.0},
	{'skin': 162, 'name': 'Slimy Farmer', 'gravity': 0.008, 'CJ_run': True, 'description': 'Slimy Farmer ', 'contact_instant_zombie': False, 'health': 800.0},
]

DEFAULT_WEAPONS = [
	[24,100],
	[25, 100]
]
HUMAN_CLASSES = [
	{'skin': 186, 'name': 'Distressed Businessman', 'weapons': DEFAULT_WEAPONS, 'description': 'A normal ped', 'health': 100.0},
	{'skin': 280, 'name': 'Cop', 'weapons': DEFAULT_WEAPONS, 'description': 'A normal ped', 'health': 100.0, 'armour': 100.0},

]

#alle entities extend from this
class GenericEntity(CoreServer.BaseEntity):
	def __init__(self):
		print("base entity")
		self.IsBot = False

class PlayerEntity(GenericEntity):
	def __init__(self, connection):
		super(PlayerEntity, self).__init__()

		self.connection = connection
		print("Make SAMPEntity: {}".format(connection))
	def OnEnterWorld(self):
		print("Enter world!")
		self.connection.SendMessage(0xFFFFFFFF, ("Welcome to the Python server, your IP is: {}:{}".format(self.connection.GetIP(), self.connection.GetPort())))
class SAMPHandler(CoreServer.Connection, CoreServer.CommandHandler):
	def __init__(self):
		SAMP.SetNumSpawnClasses(self, 5)
		self.Entity = PlayerEntity(self)
		self.SetEntity(self.Entity)
	
		self.Entity.Health = 100
	def OnSpawnSelect(self, index):
		print("Got class selection: {}".format(index))
		SAMP.ShowGameText(self, "Some text", 5000, 6)
		self.SendMessage(0xFF00FFFF, "blah blah")
	def OnChatMessagae(self, message):
		print("message: {}".format(message))

def pickup_event(pickup_entity, player_entity):
	print("asdasd")

class SAMP3DTextEntity(GenericEntity):
	def __init__(self):
		print("asdasd")

class VehicleEntity(GenericEntity):
	def __init__(self):
		print("new veh entity")	

class PickupEntity(GenericEntity):
	def __init__(self):
		print("Pickup entity")


CoreServer.SetConnectionHandler(SAMPHandler)
SAMP.SetPickupEntity(PickupEntity)
SAMP.Set3DTextLabelEntity(SAMP3DTextEntity)
SAMP.SetVehicleEntity(VehicleEntity)
