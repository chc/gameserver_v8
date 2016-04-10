import CoreServer
import Frontend

class GenericEntity(CoreServer.BaseEntity):
	def __init__(self):
		print("base entity")

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
				'position': [1529.6,-1691.2,13.3],
				'model': 111
			})

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
		self.Entity.Position = [0.0,0.0,100.0]
		#self.Entity.Weapons = {24: 50, 31: 1000}
		#self.Entity.Weapons[24] += 100
		self.Entity.Model = 6
		self.SendMessage(0xFF00FFFF, "Set stuff")
	def handle_testdlg(self, string):
		Frontend.CreateModal(self, {'title': 'Test Dialog', 'message': 'Please enter a string', 'type': Frontend.INPUT_BOX, 'buttons': ['Ok', 'Cancel'], 'callback': self.handle_dialog, 'extra': 'anything'})
	def __init__(self):
		self.Entity = PlayerEntity(self)
		self.SetEntity(self.Entity)
		
		self.Entity.Health = 100
		samphandler_command_table = [
			{'primary_command': 'pm', 'aliases': ['privatemessage', 'privmsg'], 'function': self.handle_pmcmd},
			{'primary_command': 'dialog', 'aliases': ['dlg'], 'function': self.handle_testdlg},
			{'primary_command': 'test', 'aliases': ['blah'], 'function': self.handle_testcmd},
			{'primary_command': 'spawn', 'function': self.Entity.handle_spawncmd},

		] #commands which are usable at any user state
		print("new connection {}, IP {}:{}".format(self,self.GetIP(),self.GetPort()))
		self.RegisterCommands(samphandler_command_table)

		#self.Entity.PutInWorld({
		#	'position': [1529.6, -1691.2, 13.3],
		#	'rotation': 90.0,
		#	'skin': 111
		#	})


CoreServer.SetConnectionHandler(SAMPHandler)