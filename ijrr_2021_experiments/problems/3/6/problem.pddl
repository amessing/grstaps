(define (problem ijrr_6)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street8 street25 - street
		building16 building26 building35 - building
	)
	(:init
		(atLocation survivor0 street25)
		(atLocation survivor1 building16)
		(onFire building26)
		(underRubble street8)
		(underRubble building35)
		(needsRepair building26)
		(needsRepair building35)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street8))
		(not (needsRepair building26))
		(not (needsRepair building35))
	))
	(:metric minimize total-time)
)
