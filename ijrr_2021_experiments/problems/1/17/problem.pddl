(define (problem ijrr_17)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street9 street57 - street
		building3 building4 building13 building33 - building
	)
	(:init
		(atLocation survivor0 street57)
		(atLocation survivor1 building4)
		(onFire building3)
		(onFire building33)
		(underRubble street9)
		(underRubble building13)
		(needsRepair building3)
		(needsRepair building13)
		(needsRepair building33)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street9))
		(not (needsRepair building3))
		(not (needsRepair building13))
		(not (needsRepair building33))
	))
	(:metric minimize total-time)
)
