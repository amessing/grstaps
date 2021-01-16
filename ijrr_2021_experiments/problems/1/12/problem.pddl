(define (problem ijrr_12)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street6 street43 - street
		building15 building16 building33 building36 - building
	)
	(:init
		(atLocation survivor0 street6)
		(atLocation survivor1 building15)
		(onFire building16)
		(onFire building33)
		(underRubble street43)
		(underRubble building36)
		(needsRepair building16)
		(needsRepair building33)
		(needsRepair building36)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street43))
		(not (needsRepair building16))
		(not (needsRepair building33))
		(not (needsRepair building36))
	))
	(:metric minimize total-time)
)
