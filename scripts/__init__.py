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


COLOUR_UI_BACKGROUND = 0x660000FF
COLOUR_UI_PRIMARY_TEXT = 0xFF00FFFF
COLOUR_UI_CONFIRMATION_TEXT = 0x00FF00FF
COLOUR_UI_BUTTON_BACKGROUND = 0x000000FF
class WelcomeUI():
	def __init__(self):
		self.PLAY_BUTTON_FONT_OPTIONS = {'style': SAMP.FRONTEND_FONT_GTASA_CLEAN, 'text': 'Enter Game Play', 'proportional': True, 'alignment': Frontend.TEXT_ALIGNMENT_LEFT}
		self.play_button = Frontend.CreateUIElement({'font': self.PLAY_BUTTON_FONT_OPTIONS, 'x': 200.0, 'y': 150.0, 'selectable': True, 'box': True, 'box_width': 300.0, 'box_height': 40.0, 'box_colour': COLOUR_UI_BACKGROUND})
				
		self.NEW_PLAYER_FONT_OPTIONS ={'style': SAMP.FRONTEND_FONT_GTASA_CLEAN, 'text': 'I\'m a new player, and want the tutorial', 'proportional': True, 'alignment': Frontend.TEXT_ALIGNMENT_LEFT} #
		self.tutorial_button = Frontend.CreateUIElement({'font': self.NEW_PLAYER_FONT_OPTIONS, 'x': 400.0, 'y': 150.0, 'selectable': True, 'box': True, 'box_width': 500.0, 'box_height': 40.0, 'box_colour': COLOUR_UI_BACKGROUND})

		self.BACKGROUND_FONT_OPTIONS = {'width': 1000.0, 'height': 1000.0}
		self.background = Frontend.CreateUIElement({'box_colour': COLOUR_UI_BACKGROUND, 'box': True, 'x': 0.0, 'y': 0.0, 'box_width': 1000.0, 'box_height': 1000.0, 'font': self.BACKGROUND_FONT_OPTIONS})

		self.HEADER_FONT_OPTIONS = {'text': 'Welcome to West Coast Zombie Apocalypse!', 'style': SAMP.FRONTEND_FONT_GTASA_CORONA, 'colour': COLOUR_UI_PRIMARY_TEXT, 'proportional': True}
		self.header = Frontend.CreateUIElement({'font': self.HEADER_FONT_OPTIONS, 'x': 200.0, 'y': 100.0})
		

		self.UIElements = [self.background, self.header, self.play_button, self.tutorial_button]
	def OnClick(self, conn, element):
		if element == self.play_button:
			#conn.Entity.Spawn({'model': 211, 'position': [0,0,5]})
			self.Hide(conn)
			the_class_sel_ui.Display(conn)
		conn.SendMessage(0xFF00FFFF, "Got click: {}".format(element))
	def Display(self, conn):
		Frontend.DisplayUIElements(conn, self.UIElements)
		Frontend.ActivateMouse(conn, {'enabled': True, 'hover_colour': COLOUR_UI_CONFIRMATION_TEXT, 'callback': self.OnClick})
	def Hide(self, conn):
		Frontend.ActivateMouse(conn, {'enabled': False})
		Frontend.HideUIElements(conn, self.UIElements)
class LabeledUIPreviewElement():
	#"label" dict keys - [{'colour': 0xFF00FFFF, 'text': 'label string']
	def __init__(self, options):
		self.IsSetup = True
		if not 'rotation' in options:
			options['rotation'] = [180,180,0]
		if not 'colours' in options:
			options['colours'] = [0, 0]
		if not 'zoom' in options:
			options['zoom'] = 1.0

		self.MODEL_PREVIEW_FONT_OPTIONS = {'colour': COLOUR_UI_PRIMARY_TEXT, 'style': SAMP.FRONTEND_FONT_GTASA_DFF_MODEL, 'proportional': True, 'model': options['model'], 'model_colours': options['colours'], 'model_camera_zoom': options['zoom'], 'rotation': options['rotation'], 'alignment': Frontend.TEXT_ALIGNMENT_CENTER}
		self.model_preview = Frontend.CreateUIElement({'font': self.MODEL_PREVIEW_FONT_OPTIONS, 'x': options['x'], 'y': options['y'], 'selectable': True, 'box': True, 'box_width': 75.0, 'box_height': 75.0, 'box_colour': COLOUR_UI_BACKGROUND})
	def Display(self, conn):
		print("asdasd")
		Frontend.DisplayUIElements(conn, [self.model_preview])
	def HandleClick(self, conn, element):
		if element == self.model_preview:
			conn.SendMessage(0xFFFFFFFF, "asdasdasd")
			return True
		return False
class ClassSelectionUI():
	def __init__(self):

		#menu stage 1 - class selection
		self.PLAY_BUTTON_FONT_OPTIONS = {'style': SAMP.FRONTEND_FONT_GTASA_CLEAN, 'text': 'Zombie', 'proportional': True, 'alignment': Frontend.TEXT_ALIGNMENT_LEFT}
		self.zombie_button = Frontend.CreateUIElement({'font': self.PLAY_BUTTON_FONT_OPTIONS, 'x': 200.0, 'y': 150.0, 'selectable': True, 'box': True, 'box_width': 300.0, 'box_height': 40.0, 'box_colour': COLOUR_UI_BACKGROUND})
				
		self.NEW_PLAYER_FONT_OPTIONS ={'style': SAMP.FRONTEND_FONT_GTASA_CLEAN, 'text': 'Human', 'proportional': True, 'alignment': Frontend.TEXT_ALIGNMENT_LEFT} #
		self.human_button = Frontend.CreateUIElement({'font': self.NEW_PLAYER_FONT_OPTIONS, 'x': 400.0, 'y': 150.0, 'selectable': True, 'box': True, 'box_width': 500.0, 'box_height': 40.0, 'box_colour': COLOUR_UI_BACKGROUND})

		self.BACKGROUND_FONT_OPTIONS = {'width': 1000.0, 'height': 1000.0}
		self.background = Frontend.CreateUIElement({'box_colour': COLOUR_UI_BACKGROUND, 'box': True, 'x': 0.0, 'y': 0.0, 'box_width': 1000.0, 'box_height': 1000.0, 'font': self.BACKGROUND_FONT_OPTIONS})

		self.HEADER_FONT_OPTIONS = {'text': 'Select your desired class', 'style': SAMP.FRONTEND_FONT_GTASA_CORONA, 'colour': COLOUR_UI_PRIMARY_TEXT, 'proportional': True}
		self.header = Frontend.CreateUIElement({'font': self.HEADER_FONT_OPTIONS, 'x': 200.0, 'y': 100.0})

		self.ClassSelectionUI = [self.background, self.header, self.human_button, self.zombie_button]


		#menu stage 2 - weapon selection
		self.primary_weapon_uielm = LabeledUIPreviewElement({'x': 100.0, 'y': 150.0, 'model': 348, 'title': 'Gun', 'footer': 'heii', 'zoom': 1.5 })
		
		self.secondary_weapon_uielm = LabeledUIPreviewElement({'x': 250.0, 'y': 150.0, 'model': 353, 'title': 'Gun', 'footer': 'heii', 'zoom': 1.5 })
		self.melee_weapon_uielm = LabeledUIPreviewElement({'x': 450.0, 'y': 150.0, 'model': 334, 'title': 'Gun', 'footer': 'heii', 'zoom': 1.5 })

		self.stage_2_menu = [self.primary_weapon_uielm, self.secondary_weapon_uielm, self.melee_weapon_uielm]
	def OnClick(self, conn, element):
		if element == None:
			return 0
		conn.SendMessage(0xFF00FFFF, "class sel click: {}".format(element))
		for ui_elem in  self.stage_2_menu:
			if ui_elem.HandleClick(conn, element):
				conn.SendMessage(COLOUR_UI_PRIMARY_TEXT,"showing ui element")
				return
		if element  == self.human_button:
			conn.Entity.IsHuman = True
		else:
			conn.Entity.IsHuman = False
		self.Hide(conn)
		self.EnterWeaponSelMenu(conn)
	def EnterWeaponSelMenu(self, conn):
		conn.SendMessage(0xFFFFFFFF, "Enter weapon sel menu")
		for ui_elem in self.stage_2_menu:
			ui_elem.Display(conn)
		Frontend.ActivateMouse(conn, {'enabled': True, 'hover_colour': COLOUR_UI_CONFIRMATION_TEXT, 'callback': self.OnClick})
	def Display(self, conn):
		Frontend.DisplayUIElements(conn, self.ClassSelectionUI)
		Frontend.ActivateMouse(conn, {'enabled': True, 'hover_colour': COLOUR_UI_CONFIRMATION_TEXT, 'callback': self.OnClick})
	def Hide(self, conn):
		Frontend.ActivateMouse(conn, {'enabled': False})
		Frontend.HideUIElements(conn, self.ClassSelectionUI)

the_class_sel_ui = ClassSelectionUI()
the_welcome_ui = WelcomeUI()
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
		self.GotClassSelect = False
		SAMP.SetNumSpawnClasses(self, 5)
		self.Entity = PlayerEntity(self)
		self.SetEntity(self.Entity)
	
		self.Entity.Health = 100
	def OnSpawnSelect(self, index):
		print("Got class selection: {}".format(index))
		SAMP.ShowGameText(self, "Some text", 5000, 6)
		self.SendMessage(0xFF00FFFF, "blah blah")

		if not self.GotClassSelect:
			self.GotClassSelect = True
			the_welcome_ui.Display(self)
	def OnChatMessage(self, message):
		print("Got msg!")

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
