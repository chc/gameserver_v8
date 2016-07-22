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
WEAPON_GTASA_DATA = {
	0: {'model': 1575, 'ammo': 1, 'name': 'Fist'},
	1: {'model': 331, 'ammo': 1, 'name': 'Brass Knuckles'},
	2: {'model': 333, 'ammo': 1, 'name': 'Golf Club'}, 
	3: {'model': 334, 'ammo': 1, 'name': 'Nightstick'}, 
	4: {'model': 335, 'ammo': 1, 'name': 'Knife'}, 
	5: {'model': 336, 'ammo': 1, 'name': 'Baseball Bat'},
	6: {'model': 337, 'ammo': 1, 'name': 'Shovel'},
	7: {'model': 338, 'ammo': 1, 'name': 'Pool cue'},
	8: {'model': 339, 'ammo': 1, 'name': 'Katana'},
	9: {'model': 341, 'ammo': 1, 'name': 'Chainsaw'},
	10: {'model': 321, 'ammo': 1, 'name': 'Purple Dildo'},
	11: {'model': 322, 'ammo': 1, 'name': 'Dildo'},
	12: {'model': 323, 'ammo': 1, 'name': 'Vibrator'},
	13: {'model': 324, 'ammo': 1, 'name': 'Small Vibrator'},
	14: {'model': 325, 'ammo': 1, 'name': 'Flowers'},
	15: {'model': 326, 'ammo': 1, 'name': 'Cane'},
	16: {'model': 342, 'ammo': 10, 'name': 'Grenade'},
	17: {'model': 343, 'ammo': 15, 'name': 'Tear Gas'},
	18: {'model': 344, 'ammo': 10, 'name': 'Molotov Cocktail'},
	22: {'model': 346, 'ammo': 250, 'name': '9mm'},
	23: {'model': 347, 'ammo': 250, 'name': 'Silenced 9mm'},
	24: {'model': 348, 'ammo': 250, 'name': 'Deagle'},
	25: {'model': 349, 'ammo': 1000, 'name': 'Shotgun'},
	26: {'model': 350, 'ammo': 1000, 'name': 'Sawnoff Shotgun'},
	27: {'model': 351, 'ammo': 1000, 'name': 'SPAS12'},
	28: {'model': 352, 'ammo': 5000, 'name': 'UZI'},
	29: {'model': 353, 'ammo': 5000, 'name': 'MP5'},
	30: {'model': 355, 'ammo': 5000, 'name': 'AK-47'},
	31: {'model': 356, 'ammo': 5000, 'name': 'M4'},
	32: {'model': 372, 'ammo': 5000, 'name': 'Tec9'},
	33: {'model': 357, 'ammo': 5000, 'name': 'Country Rifle'},
	34: {'model': 358, 'ammo': 5000, 'name': 'Sniper Rifle'},
	35: {'model': 359, 'ammo': 5000, 'name': 'RPG'},
	36: {'model': 360, 'ammo': 5000, 'name': 'HS Rocket'},
	37: {'model': 361, 'ammo': 5000, 'name': 'Flamethrower'},
	38: {'model': 362, 'ammo': 5000, 'name': 'Minigun'},
	39: {'model': 363, 'ammo': 1, 'name': 'Satchel Charge'},
	40: {'model': 364, 'ammo': 1, 'name': 'Detonator'},
	41: {'model': 365, 'ammo': 9000, 'name': 'Spraycan'},
	42: {'model': 366, 'ammo': 9000, 'name': 'Fire Extinguisher'},
	43: {'model': 367, 'ammo': 9000, 'name': 'Camera'},
	44: {'model': 368, 'ammo': 1, 'name': 'Night Vision Goggles'},
	45: {'model': 369, 'ammo': 1, 'name': 'Thermal Goggles'},
	46: {'model': 371, 'ammo': 1, 'name': 'parachute'},
}

WEAPON_CLASS_PRIMARY = {'class_id': 0, 'weapons': [
	[30, 100],
	[31, 100],
	[33, 100],
	[34, 100],
	[35, 100],
	[37, 1000],
	[38, 1000],
]}
WEAPON_CLASS_SECONDARY = {'class_id': 1, 'weapons': [
	[22, 100],
	[23, 100],
	[24, 100],
	[25, 100],
	[26, 100],
	[27, 100],
	[28, 100],
	[29, 100],
	[32, 100],
]}
WEAPON_CLASS_MELEE = {'class_id': 2, 'weapons': [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18]}

COLOUR_UI_BACKGROUND = 0x660000FF
COLOUR_UI_PRIMARY_TEXT = 0xFF00FFFF
COLOUR_UI_CONFIRMATION_TEXT = 0x00FF00FF
COLOUR_UI_BUTTON_BACKGROUND = 0xFF000000

#gameplay HUD
class HumanHUD():
	def __init__(self, connection):
		self.connection = connection
		self.time_left_label = Frontend.CreateUIElement({'font': {'style': SAMP.FRONTEND_FONT_GTASA_CLEAN, 'text': 'Time Left: 0:00', 'proportional': True, 'alignment': Frontend.TEXT_ALIGNMENT_LEFT}, 
													 'x': 30.0, 'y': 301.0, 'selectable': False, 'box': False, 'box_width': 280.0, 'box_height': 40.0, 'box_colour': COLOUR_UI_PRIMARY_TEXT})

		self.hud_items = [self.time_left_label]
	def Display(self):
		Frontend.DisplayUIElements(self.connection, self.hud_items)



#initial gameplay ui
class WelcomeUI():
	def __init__(self, connection):
		self.connection = connection
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
			self.Hide()
			self.connection.class_sel_ui.Display()
		self.connection.SendMessage(0xFF00FFFF, "Got click: {}".format(element))
	def Display(self):
		Frontend.DisplayUIElements(self.connection, self.UIElements)
		Frontend.ActivateMouse(self.connection, {'enabled': True, 'hover_colour': COLOUR_UI_CONFIRMATION_TEXT, 'callback': self.OnClick})
	def Hide(self):
		Frontend.ActivateMouse(self.connection, {'enabled': False})
		Frontend.HideUIElements(self.connection, self.UIElements)
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

		if 'identifier' in options:
			self.identifier = options['identifier']

		if 'connection' in options:
			self.connection = options['connection']

		if 'callback' in options:
			self.callback = options['callback']

		self.MODEL_PREVIEW_FONT_OPTIONS = {'colour': COLOUR_UI_PRIMARY_TEXT, 'style': SAMP.FRONTEND_FONT_GTASA_DFF_MODEL, 'proportional': True, 'model': options['model'], 'model_colours': options['colours'], 'model_camera_zoom': options['zoom'], 'rotation': options['rotation'], 'alignment': Frontend.TEXT_ALIGNMENT_CENTER}
		self.model_preview = Frontend.CreateUIElement({'font': self.MODEL_PREVIEW_FONT_OPTIONS, 'x': options['x'], 'y': options['y'], 'selectable': True, 'box': True, 'box_width': 75.0, 'box_height': 75.0, 'box_colour': COLOUR_UI_BACKGROUND})

		self.footer_text = None
		if 'footer' in options:
			self.footer_text = options['footer']
		print("Set footer to: {}".format(self.footer_text))


		self.HEADER_FONT_OPTIONS = {'text': 'Welcome to West Coast Zombie Apocalypse!', 'style': SAMP.FRONTEND_FONT_GTASA_CORONA, 'colour': COLOUR_UI_PRIMARY_TEXT, 'proportional': True}
		self.header = Frontend.CreateUIElement({'font': self.HEADER_FONT_OPTIONS, 'x': 200.0, 'y': 100.0})


		self.FOOTER_FONT_OPTIONS = {'text': self.footer_text, 'style': SAMP.FRONTEND_FONT_GTASA_CLEAN, 'colour': COLOUR_UI_BUTTON_BACKGROUND, 'proportional': True, 'height': 1.12, 'width': 0.480000/2}
		self.footer_element = Frontend.CreateUIElement({'font': self.FOOTER_FONT_OPTIONS, 'x': options['x'], 'y': options['y']+ 65.0, 'box_colour': COLOUR_UI_BACKGROUND})
	def Display(self):
		Frontend.DisplayUIElements(self.connection, [self.footer_element, self.model_preview])
	def SetModel(self, model):
		Frontend.SetUIElementModel({'model': model, 'element': self.model_preview})
	def SetFooter(self, text):
		Frontend.SetUIElementText({'text': text, 'element': self.footer_element})
	def HandleClick(self, conn, element):
		if element == self.model_preview:
			return True
		return False
	def OwnsElement(self, element):
		return element == self.model_preview
	def Hide(self):
		Frontend.HideUIElements(self.connection, [self.footer_element, self.model_preview])
class WeaponSelectUI():
	MAX_ITEM_ROWS = 5
	MAX_ITEM_COLUMNS = 5
	def __init__(self, class_info, connection):
		self.weapon_buttons = {}
		index = 0
		self.connection = connection
		for item in class_info['weapons']:
			if type(item) is list:
				self.weapon_buttons[item[0]] = self.CreateWeaponTD(WEAPON_GTASA_DATA[item[0]], index, item[0], connection)
			else:
				self.weapon_buttons[item] = self.CreateWeaponTD(WEAPON_GTASA_DATA[item], index, item, connection)
			index += 1
	def CreateWeaponTD(self, item, index, wep_id, connection):
		options = {}
		x_index = index % 5
		y_index = ((int)(index / 5))
		options['model'] = item['model']
		options['x'] = 175.0 + (x_index * 80.0)
		y_pos = 15.0
		y_pos += (y_index * 80.0)
		options['y'] = y_pos

		options['zoom'] = 1.5

		options['identifier'] = wep_id
		options['connection'] = connection

		options['footer'] = WEAPON_GTASA_DATA[wep_id]['name']
		element = LabeledUIPreviewElement(options)
		return element
	def OnClick(self, conn, element):
		for wep_id in self.weapon_buttons:
			if self.weapon_buttons[wep_id].OwnsElement(element):
				#conn.SendMessage(0xFFFFFFFF, "Clicked: {}".format(WEAPON_GTASA_DATA[wep_id]['name']))
				self.click_callback(conn, self.index, WEAPON_GTASA_DATA[wep_id], wep_id)
				self.Hide()
				return
	def Display(self, callback):
		for wep_id in self.weapon_buttons:
			self.weapon_buttons[wep_id].Display()
		self.click_callback = callback
		self.connection.SendMessage(0xFFFFFFFF, "weapon select display")
		Frontend.ActivateMouse(self.connection, {'enabled': True, 'hover_colour': COLOUR_UI_CONFIRMATION_TEXT, 'callback': self.OnClick})
	def Hide(self):
		for wep_id in self.weapon_buttons:
			self.weapon_buttons[wep_id].Hide()

class ClassSelectionUI():
	def __init__(self, connection):

		self.connection = connection

		#menu stage 1 - class selection
		self.PLAY_BUTTON_FONT_OPTIONS = {'style': SAMP.FRONTEND_FONT_GTASA_CLEAN, 'text': 'Zombie', 'proportional': True, 'alignment': Frontend.TEXT_ALIGNMENT_LEFT}
		self.zombie_button = Frontend.CreateUIElement({'connection': connection, 'font': self.PLAY_BUTTON_FONT_OPTIONS, 'x': 200.0, 'y': 150.0, 'selectable': True, 'box': True, 'box_width': 300.0, 'box_height': 40.0, 'box_colour': COLOUR_UI_BACKGROUND})
				
		self.NEW_PLAYER_FONT_OPTIONS ={'style': SAMP.FRONTEND_FONT_GTASA_CLEAN, 'text': 'Human', 'proportional': True, 'alignment': Frontend.TEXT_ALIGNMENT_LEFT} #
		self.human_button = Frontend.CreateUIElement({'connection': connection, 'font': self.NEW_PLAYER_FONT_OPTIONS, 'x': 400.0, 'y': 150.0, 'selectable': True, 'box': True, 'box_width': 500.0, 'box_height': 40.0, 'box_colour': COLOUR_UI_BACKGROUND})

		self.BACKGROUND_FONT_OPTIONS = {'width': 1000.0, 'height': 1000.0}
		self.background = Frontend.CreateUIElement({'connection': connection, 'box_colour': COLOUR_UI_BACKGROUND, 'box': True, 'x': 0.0, 'y': 0.0, 'box_width': 1000.0, 'box_height': 1000.0, 'font': self.BACKGROUND_FONT_OPTIONS})

		self.HEADER_FONT_OPTIONS = {'text': 'Select your desired class', 'style': SAMP.FRONTEND_FONT_GTASA_CORONA, 'colour': COLOUR_UI_PRIMARY_TEXT, 'proportional': True}
		self.header = Frontend.CreateUIElement({'connection': connection, 'font': self.HEADER_FONT_OPTIONS, 'x': 200.0, 'y': 100.0})

		self.ui = [self.background, self.header, self.human_button, self.zombie_button]


		#menu stage 2 - weapon selection
		self.primary_weapon_uielm = LabeledUIPreviewElement({'connection': connection, 'x': 100.0, 'y': 150.0, 'model': 348, 'title': 'Gun', 'footer': 'Empty', 'zoom': 1.5, 'callback': self.OnPrimaryWeaponSelect })
		self.primary_weapon_uielm.index = 0
		
		self.secondary_weapon_uielm = LabeledUIPreviewElement({'connection': connection, 'x': 250.0, 'y': 150.0, 'model': 353, 'title': 'Gun', 'footer': 'Empty', 'zoom': 1.5, 'callback': self.OnPrimaryWeaponSelect })
		self.secondary_weapon_uielm.index = 1

		self.melee_weapon_uielm = LabeledUIPreviewElement({'connection': connection, 'x': 450.0, 'y': 150.0, 'model': 334, 'title': 'Gun', 'footer': 'Empty', 'zoom': 1.5, 'callback': self.OnPrimaryWeaponSelect })
		self.melee_weapon_uielm.index = 2

		self.SPAWN_BUTTON_FONT_OPTIONS = {'style': SAMP.FRONTEND_FONT_GTASA_CLEAN, 'text': 'Spawn', 'proportional': True, 'alignment': Frontend.TEXT_ALIGNMENT_LEFT} 
		self.spawn_button = Frontend.CreateUIElement({'connection': connection, 'font': self.SPAWN_BUTTON_FONT_OPTIONS, 'x': 100.0, 'y': 400.0, 'selectable': True, 'box': True, 'box_width': 150.0, 'box_height': 40.0, 'box_colour': COLOUR_UI_BACKGROUND})

		self.RETURN_BUTTON_FONT_OPTIONS = {'style': SAMP.FRONTEND_FONT_GTASA_CLEAN, 'text': 'Return to menu', 'proportional': True, 'alignment': Frontend.TEXT_ALIGNMENT_LEFT} 
		self.return_button = Frontend.CreateUIElement({'connection': connection, 'font': self.RETURN_BUTTON_FONT_OPTIONS, 'x': 250.0, 'y': 400.0, 'selectable': True, 'box': True, 'box_width': 385.0, 'box_height': 40.0, 'box_colour': COLOUR_UI_BACKGROUND})

		self.stage_2_menu = [self.primary_weapon_uielm, self.secondary_weapon_uielm, self.melee_weapon_uielm]

		self.action_buttons = [self.spawn_button, self.return_button]

		self.stage = 0

		self.weapon_select_ui_elements = []

		self.selected_weapons = {}

		the_weapon_select_menus = [WEAPON_CLASS_PRIMARY, WEAPON_CLASS_SECONDARY, WEAPON_CLASS_MELEE]
		index = 0
		for weapon_data in the_weapon_select_menus:
			weapon_ui = WeaponSelectUI(weapon_data, self.connection)
			weapon_ui.index = index
			index += 1
			self.weapon_select_ui_elements.append(weapon_ui)
	def OnPrimaryWeaponSelect(self, conn, index, weapon, wep_id):
		print("CB called.. {} {} {}".format(index, weapon, wep_id))
		self.stage_2_menu[index].SetModel(weapon['model'])
		self.stage_2_menu[index].SetFooter(weapon['name'])
		self.selected_weapons[index] = weapon
		self.selected_weapons[index]['wep_id'] = wep_id
		self.Hide()
		self.EnterWeaponSelMenu()
	def OnClick(self, conn, element):
		if element == None:
			return 0
		self.connection.SendMessage(0xFF00FFFF, "class sel click: {}".format(element))
		clicked_element = None
		for ui_elem in  self.stage_2_menu:
			if ui_elem.HandleClick(self.connection, element):
				clicked_element = ui_elem
		if element == self.spawn_button:
			self.Hide()
			print("Weapons: {}".format(self.selected_weapons))
			weapons = []
			for slot, weapon in self.selected_weapons.items():
				weapons.append({'id': weapon['wep_id'], 'ammo': weapon['ammo']})

			self.connection.Entity.Spawn({'position': THE_SPAWN_COORDS, 'model': 15, 'weapons': weapons})
			return
		if clicked_element != None:
			conn.SendMessage(COLOUR_UI_PRIMARY_TEXT,"showing ui element")
			self.stage = 2
			self.Hide()
			print("Display: weapon set index: {} {}".format(clicked_element.index, self.stage))
			self.weapon_select_ui_elements[clicked_element.index].Display(clicked_element.callback)
			return
			#clicked_element.Display()
		if element  == self.human_button:
			self.connection.Entity.IsHuman = True
		else:
			self.connection.Entity.IsHuman = False
		self.Hide()
		self.EnterWeaponSelMenu()
	def EnterWeaponSelMenu(self):
		self.stage = 2
		self.connection.SendMessage(0xFFFFFFFF, "Enter weapon sel menu")
		for ui_elem in self.stage_2_menu:
			ui_elem.Display()
		Frontend.DisplayUIElements(self.connection, self.action_buttons)
		Frontend.ActivateMouse(self.connection, {'enabled': True, 'hover_colour': COLOUR_UI_CONFIRMATION_TEXT, 'callback': self.OnClick})
	def Display(self):
		self.stage = 1
		Frontend.DisplayUIElements(self.connection, self.ui)
		Frontend.ActivateMouse(self.connection, {'enabled': True, 'hover_colour': COLOUR_UI_CONFIRMATION_TEXT, 'callback': self.OnClick})
	def Hide(self):
		if self.stage == 2:
			for ui_elem in self.stage_2_menu:
				ui_elem.Hide()

			Frontend.HideUIElements(self.connection, self.action_buttons)
		else:
			Frontend.HideUIElements(self.connection, self.ui)
		self.stage = 0
		Frontend.ActivateMouse(self.connection, {'enabled': False})


#alle entities extend from this
class GenericEntity(CoreServer.BaseEntity):
	def __init__(self):
		print("base entity")
		self.IsBot = False

class PlayerEntity(GenericEntity):
	def __init__(self, connection):
		print("Begin make entity")
		super(PlayerEntity, self).__init__()

		self.connection = connection
		self.main_hud = HumanHUD(self.connection)
		print("Make SAMPEntity: {}".format(connection))
	def OnEnterWorld(self):
		print("Enter world!")
		self.main_hud.Display()
		self.connection.SendMessage(0xFFFFFFFF, ("Welcome to the Python server, your IP is: {}:{}".format(self.connection.GetIP(), self.connection.GetPort())))
class SAMPHandler(CoreServer.Connection, CoreServer.CommandHandler):
	def __init__(self):
		self.GotClassSelect = False
		SAMP.SetNumSpawnClasses(self, 5)
		self.Entity = PlayerEntity(self)
		self.SetEntity(self.Entity)

		self.class_sel_ui = ClassSelectionUI(self)
		self.welcome_ui = WelcomeUI(self)
	
		self.Entity.Health = 100
	def OnSpawnSelect(self, index):
		print("Got class selection: {}".format(index))
		SAMP.ShowGameText(self, "Some text", 5000, 6)
		self.SendMessage(0xFF00FFFF, "blah blah")

		if not self.GotClassSelect:
			self.GotClassSelect = True
			self.welcome_ui.Display()
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
SAMP.SetServerProperties({
		'name': 'Zombies Server', 
		'max_players': 1000, 
		'password': '123321',
		'mapname': 'Los Santos',
		'gamemode': 'Zombie',
		'rules': {
			'version': 'python-server',
			'weburl': 'http://google.com',

		}
	})
