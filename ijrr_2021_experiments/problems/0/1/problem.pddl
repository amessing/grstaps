(define (problem ijrr_1)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street32 street49 - street
		building3 building4 building13 building20 - building
	)
	(:init
		(atLocation survivor0 street32)
		(atLocation survivor1 building3)
		(onFire building13)
		(onFire building20)
		(underRubble street49)
		(underRubble building4)
		(needsRepair building4)
		(needsRepair building13)
		(needsRepair building20)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street49))
		(not (needsRepair building4))
		(not (needsRepair building13))
		(not (needsRepair building20))
	))
	(:metric minimize total-time)
)
