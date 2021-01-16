(define (problem ijrr_9)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street11 street19 - street
		building26 building31 building36 - building
	)
	(:init
		(atLocation survivor0 street19)
		(atLocation survivor1 building36)
		(onFire building26)
		(onFire building31)
		(underRubble street11)
		(underRubble building36)
		(needsRepair building26)
		(needsRepair building31)
		(needsRepair building36)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street11))
		(not (needsRepair building26))
		(not (needsRepair building31))
		(not (needsRepair building36))
	))
	(:metric minimize total-time)
)
