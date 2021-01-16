(define (problem ijrr_9)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street49 street57 - street
		building1 building2 building16 building35 - building
	)
	(:init
		(atLocation survivor0 street49)
		(atLocation survivor1 building16)
		(onFire building1)
		(onFire building35)
		(underRubble street57)
		(underRubble building2)
		(needsRepair building1)
		(needsRepair building2)
		(needsRepair building35)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street57))
		(not (needsRepair building1))
		(not (needsRepair building2))
		(not (needsRepair building35))
	))
	(:metric minimize total-time)
)
