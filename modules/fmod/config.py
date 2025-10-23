def can_build(env, platform):
	return True

def configure(env):
	pass

def get_doc_classes():
	return [
		"FMODManager",
		"FMODEventInstance",
		"FMODListener3D",
		"FMODListener2D"
	]

def get_doc_path():
	return "doc_classes"
